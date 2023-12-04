use std::vec;

use nom::types::CompleteStr;

use crate::instruction::Opcode;

use self::{
    assembler_error::AssemblerError,
    instruction_parser::AssemblerInstruction,
    program_parser::{program, Program},
    symbol::{Symbol, SymbolTable, SymbolType},
};

pub mod assembler_error;
pub mod directive_parser;
pub mod instruction_parser;
pub mod label_parser;
pub mod opcode_parsers;
pub mod operand_parsers;
pub mod program_parser;
pub mod register_parsers;
pub mod symbol;

/// Magic number that begins every bytecode file prefix. These spell out EPIE in ASCII, if you were wondering.
pub const PIE_HEADER_PREFIX: [u8; 4] = [45, 50, 49, 45];
/// Constant that determines how long the header is. There are 60 zeros left after the prefix, for later usage if needed.
pub const PIE_HEADER_LENGTH: usize = 64;

#[derive(PartialEq, Debug)]
pub enum Token {
    Op { code: Opcode },
    Register { reg_num: u8 },
    IntegerOperand { value: i32 },
    FloatOperand { value: f64 },
    LabelDeclaration { name: String },
    LabelUsage { name: String },
    Directive { name: String },
    IrString { name: String },
    Comment,
}

#[derive(Debug, PartialEq)]
pub enum AssemblerPhase {
    First,
    Second,
}

#[derive(Debug, Clone, PartialEq)]
pub enum AssemblerSection {
    Data { startting_instruction: Option<u32> },
    Code { startting_instruction: Option<u32> },
    Unknown,
}

#[derive(Debug)]
pub struct Assembler {
    /// Track phase state
    pub phase: AssemblerPhase,
    pub symbols: SymbolTable,
    /// Read-only data section constants data
    pub ro: Vec<u8>,
    /// Compiled bytedcode generated from assembly instruction
    pub bytecode: Vec<u8>,

    /// Tracks current offset of RO secton
    ro_offset: u32,

    sections: Vec<AssemblerSection>,
    current_section: Option<AssemblerSection>,

    /// Current instruction going convert to bytecode
    current_instructon: u32,

    errors: Vec<AssemblerError>,

    buf: [u8; 4],
}

impl Assembler {
    pub fn new() -> Assembler {
        Assembler {
            phase: AssemblerPhase::First,
            symbols: SymbolTable::new(),
            ro: vec![],
            bytecode: vec![],
            ro_offset: 0,
            sections: vec![],
            current_section: None,
            current_instructon: 0,
            errors: vec![],
            buf: [0; 4],
        }
    }

    pub fn assemble(&mut self, raw: &str) -> Result<Vec<u8>, Vec<AssemblerError>> {
        // pass to parser
        match program(CompleteStr(&raw)) {
            Ok((_remainder, program)) => {
                let mut assembled_program = self.write_pie_header();

                self.process_first_phase(&program);

                if !self.errors.is_empty() {
                    return Err(self.errors.clone());
                }

                if self.sections.len() != 2 {
                    println!("Required at least 2 sections.");
                    self.errors.push(AssemblerError::InsufficientSections);
                    return Err(self.errors.clone());
                }

                let mut body = self.process_second_phase(&program);

                assembled_program.append(&mut body);
                Ok(assembled_program)
            }

            Err(e) => {
                println!("There was an error assembling the code: {:?}", e);
                Err(vec![AssemblerError::ParseError {
                    error: e.to_string(),
                }])
            }
        }
    }

    fn process_first_phase(&mut self, p: &Program) {
        for i in &p.instructions {
            if i.is_label() {
                if self.current_section.is_some() {
                    self.process_label_declaration(&i);
                } else {
                    self.errors.push(AssemblerError::NoSegmentDeclarationFound {
                        instruction: self.current_instructon,
                    })
                }
            }

            if i.is_directive() {
                self.process_directive(i);
            }

            self.current_instructon += 1;
        }

        self.phase = AssemblerPhase::Second;
    }

    fn process_second_phase(&mut self, p: &Program) -> Vec<u8> {
        // reset
        self.current_instructon = 0;
        let mut program = vec![];
        for i in &p.instructions {
            if i.is_opcode() {
                let mut bytes = i.to_bytes(&self.symbols);
                program.append(&mut bytes);
            }
            if i.is_directive() {
                self.process_directive(i);
            }
            self.current_instructon += 1;
        }
        program
    }

    fn write_pie_header(&self) -> Vec<u8> {
        let mut header = vec![];
        PIE_HEADER_PREFIX
            .iter()
            .for_each(|b| header.push(b.clone()));
        header
    }

    fn process_label_declaration(&mut self, i: &AssemblerInstruction) {
        let name = match i.get_label_name() {
            Some(name) => name,
            None => {
                self.errors
                    .push(AssemblerError::StringConstantDeclaredWithoutLabel {
                        instruction: self.current_instructon,
                    });
                return;
            }
        };

        if self.symbols.has_symbol(&name) {
            self.errors.push(AssemblerError::SymbolAlreadyDeclared);
            return;
        }

        let symbol = Symbol::new(name.to_string(), SymbolType::Label);
        self.symbols.add_symbol(symbol);
    }

    fn extract_labels(&mut self, p: &Program) {
        let mut c = 0;
        for i in &p.instructions {
            if i.is_label() {
                match i.get_label_name() {
                    Some(name) => {
                        let symbol = Symbol::new(name, SymbolType::Label);
                        self.symbols.add_symbol(symbol);
                    }
                    None => {}
                }
            }
            c += 4;
        }
    }
    fn process_directive(&mut self, i: &AssemblerInstruction) {
        // First let’s make sure we have a parseable name
        let directive_name = match i.get_directive_name() {
            Some(name) => name,
            None => {
                println!("Directive has an invalid name: {:?}", i);
                return;
            }
        };

        // Now check if there were any operands.
        if i.has_operands() {
            // If it _does_ have operands, we need to figure out which directive it was
            match directive_name.as_ref() {
                // If this is the operand, we're declaring a null terminated string
                "asciiz" => {
                    self.handle_asciiz(i);
                }
                _ => {
                    self.errors.push(AssemblerError::UnknownDirectiveFound {
                        directive: directive_name.clone(),
                    });
                    return;
                }
            }
        } else {
            // If there were not any operands, (e.g., `.code`), then we know it is a section header
            self.process_section_header(&directive_name);
        }
    }

    fn process_section_header(&mut self, header_name: &str) {
        let new_section: AssemblerSection = header_name.into();
        if new_section == AssemblerSection::Unknown {
            println!(
                "Found an section header that is unknown: {:#?}",
                header_name
            );
            return;
        }
        self.sections.push(new_section.clone());
        self.current_section = Some(new_section);
    }

    /// Handle a declarration of a null-terminated string:
    /// hello: .asciiz 'Hello!'
    fn handle_asciiz(&mut self, i: &AssemblerInstruction) {
        if self.phase != AssemblerPhase::First {
            return;
        }
        match i.get_string_constant() {
            Some(s) => {
                match i.get_label_name() {
                    Some(name) => self.symbols.set_symbol_offset(&name, self.ro_offset),
                    None => {
                        // Needing a label
                        // This would be someone typing:
                        // .asciiz 'Hello'
                        println!("Found a string constant with no associated label!");
                        return;
                    }
                };
                for b in s.as_bytes() {
                    self.ro.push(*b);
                    self.ro_offset += 1;
                }
                // null terminatation bit for c-string
                self.ro.push(0);
                self.ro_offset += 1;
            }
            None => {
                println!("String constant following an .asciiz was empty");
            }
        }
    }
}

impl Default for AssemblerPhase {
    fn default() -> Self {
        AssemblerPhase::First
    }
}

impl Default for AssemblerSection {
    fn default() -> Self {
        Self::Unknown
    }
}

impl<'a> From<&'a str> for AssemblerSection {
    fn from(value: &'a str) -> Self {
        match value {
            "data" => Self::Data {
                startting_instruction: None,
            },
            "code" => Self::Code {
                startting_instruction: None,
            },
            _ => AssemblerSection::Unknown,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::vm::VM;

    #[test]
    /// Tests assembly a small but correct program
    fn test_assemble_program() {
        let mut asm = Assembler::new();
        let test_string = r"
        .data
        .code
        load $0 #100
        load $1 #1
        load $2 #0
        test: inc $0
        neq $0 $2
        jeq @test
        hlt
        ";
        let program = asm.assemble(test_string).unwrap();
        let mut vm = VM::new();
        assert_eq!(program.len(), 96);
        vm.add_bytes(program);
        assert_eq!(vm.program.len(), 96);
    }

    #[test]
    /// Simple test of data that goes into the read only section
    fn test_code_start_offset_written() {
        let mut asm = Assembler::new();
        let test_string = r"
        .data
        test1: .asciiz 'Hello'
        .code
        load $0 #100
        load $1 #1
        load $2 #0
        test: inc $0
        neq $0 $2
        jmpe @test
        hlt
        ";
        let program = asm.assemble(test_string);
        assert_eq!(program.is_ok(), true);
        let unwrapped = program.unwrap();
        assert_eq!(unwrapped[64], 6);
    }

    #[test]
    /// Tests that we can add things to the symbol table
    fn test_symbol_table() {
        let mut sym = SymbolTable::new();
        let new_symbol = Symbol::new_with_offset("test".to_string(), SymbolType::Label, 12);
        sym.add_symbol(new_symbol);
        assert_eq!(sym.symbols.len(), 1);
        let v = sym.symbol_value("test");
        assert_eq!(true, v.is_some());
        let v = v.unwrap();
        assert_eq!(v, 12);
        let v = sym.symbol_value("does_not_exist");
        assert_eq!(v.is_some(), false);
    }

    #[test]
    /// Simple test of data that goes into the read only section
    fn test_ro_data_asciiz() {
        let mut asm = Assembler::new();
        let test_string = r"
        .data
        test: .asciiz 'This is a test'
        .code
        ";
        let program = asm.assemble(test_string);
        assert_eq!(program.is_ok(), true);
    }

    #[test]
    /// Simple test of data that goes into the read only section
    fn test_ro_data_i32() {
        let mut asm = Assembler::new();
        let test_string = r"
        .data
        test: .integer #300
        .code
        ";
        let program = asm.assemble(test_string);
        assert_eq!(program.is_ok(), true);
    }

    #[test]
    /// This tests that a section name that isn't `code` or `data` throws an error
    fn test_bad_ro_data() {
        let mut asm = Assembler::new();
        let test_string = r"
        .code
        test: .asciiz 'This is a test'
        .wrong
        ";
        let program = asm.assemble(test_string);
        assert_eq!(program.is_ok(), false);
    }

    #[test]
    /// Tests that code which does not declare a segment first does not work
    fn test_first_phase_no_segment() {
        let mut asm = Assembler::new();
        let test_string = "hello: .asciiz 'Fail'";
        let result = program(CompleteStr(test_string));
        assert_eq!(result.is_ok(), true);
        let (_, mut p) = result.unwrap();
        asm.process_first_phase(&mut p);
        assert_eq!(asm.errors.len(), 1);
    }

    #[test]
    /// Tests that code inside a proper segment works
    fn test_first_phase_inside_segment() {
        let mut asm = Assembler::new();
        let test_string = r"
        .data
        test: .asciiz 'Hello'
        ";
        let result = program(CompleteStr(test_string));
        assert_eq!(result.is_ok(), true);
        let (_, mut p) = result.unwrap();
        asm.process_first_phase(&mut p);
        assert_eq!(asm.errors.len(), 0);
    }
}
