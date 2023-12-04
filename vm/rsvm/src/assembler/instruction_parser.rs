use byteorder::{LittleEndian, WriteBytesExt};
use nom::opt;
use nom::types::CompleteStr;

use super::label_parser::label_declaration;
use super::opcode_parsers::opcode;
use super::operand_parsers::operand;

use super::symbol::SymbolTable;
use super::Token;

#[derive(Debug, PartialEq)]
pub struct AssemblerInstruction {
    pub opcode: Option<Token>,
    pub label: Option<Token>,
    pub directive: Option<Token>,
    pub operand1: Option<Token>,
    pub operand2: Option<Token>,
    pub operand3: Option<Token>,
}

named!(pub instruction_combined<CompleteStr, AssemblerInstruction>,
    do_parse!(
        l: opt!(label_declaration) >>
        o: opcode >>
        o1: opt!(operand) >>
        o2: opt!(operand) >>
        o3: opt!(operand) >>
        (
            AssemblerInstruction{
                opcode: Some(o),
                label: l,
                directive: None,
                operand1: o1,
                operand2: o2,
                operand3: o3
            }
        )
    )
);

impl AssemblerInstruction {
    pub fn to_bytes(&self, symbols: &SymbolTable) -> Vec<u8> {
        let mut ret = vec![];
        match self.opcode {
            Some(Token::Op { code }) => match code {
                _ => {
                    ret.push(code.into());
                }
            },
            _ => {
                println!("Non-opcode found in opcode field! ");
                std::process::exit(1);
            }
        }

        for operand in &[&self.operand1, &self.operand2, &self.operand3] {
            if let Some(token) = operand {
                AssemblerInstruction::extract_operand(token, &mut ret, symbols);
            }
        }
        while ret.len() < 4 {
            ret.push(0)
        }

        ret
    }

    fn extract_operand(token: &Token, ret: &mut Vec<u8>, symbols: &SymbolTable) {
        match token {
            Token::Register { reg_num } => {
                ret.push(*reg_num);
            }
            Token::IntegerOperand { value } => {
                // let raw = *value as u16;
                // let byte1 = raw; // low
                // let byte2 = raw >> 8; // high
                // ret.push(byte2 as u8);
                // ret.push(byte1 as u8);

                let mut wtr = vec![];
                wtr.write_i16::<LittleEndian>(*value as i16).unwrap();
                ret.push(wtr[1]);
                ret.push(wtr[0]);
            }
            Token::LabelUsage { name } => match symbols.symbol_value(&name) {
                Some(v) => {
                    let mut wtr = vec![];
                    wtr.write_u32::<LittleEndian>(v).unwrap();
                    ret.push(wtr[1]);
                    ret.push(wtr[0]);
                }
                None => {
                    error!("No value found for {:?}", name);
                }
            },
            _ => {
                println!("Opcode found in operand field: {:?}", token);
                // std::process::exit(1);
            }
        }
    }

    pub fn is_label(&self) -> bool {
        match &self.label {
            Some(label) => match label {
                Token::LabelDeclaration { name: _ } => true,
                Token::LabelUsage { name: _ } => true,
                _ => false,
            },
            None => false,
        }
    }

    pub fn is_directive(&self) -> bool {
        self.label.is_some()
    }

    pub fn get_label_name(&self) -> Option<String> {
        match &self.label {
            Some(label) => match label {
                Token::LabelDeclaration { name } => Some(name.clone()),
                _ => None,
            },
            None => None,
        }
    }

    pub(crate) fn get_directive_name(&self) -> Option<String> {
        match &self.directive {
            Some(directive) => match directive {
                Token::Directive { name } => Some(name.to_string()),
                _ => None,
            },
            None => None,
        }
    }

    pub(crate) fn has_operands(&self) -> bool {
        self.operand1.is_some() || self.operand2.is_some() || self.operand3.is_some()
    }

    pub(crate) fn is_opcode(&self) -> bool {
        self.opcode.is_some()
    }

    pub(crate) fn get_string_constant(&self) -> Option<String> {
        match &self.operand1 {
            Some(token) => match token {
                Token::IrString { name } => Some(name.to_string()),
                _ => None,
            },
            None => None,
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::instruction::Opcode;

    use super::*;

    #[test]
    fn test_parse_instruction_from_one() {
        let result = instruction_combined(CompleteStr("load $0 #100\n"));
        assert_eq!(
            result,
            Ok((
                CompleteStr(""),
                AssemblerInstruction {
                    opcode: Some(Token::Op { code: Opcode::LOAD }),
                    operand1: Some(Token::Register { reg_num: 0 }),
                    operand2: Some(Token::IntegerOperand { value: 100 }),
                    operand3: None,
                    label: None,
                    directive: None,
                }
            ))
        );
    }

    #[test]
    fn test_parse_instruction_from_two() {
        let result = instruction_combined("hlt\n".into());
        assert_eq!(
            result,
            Ok((
                CompleteStr("\n"),
                AssemblerInstruction {
                    opcode: Some(Token::Op { code: Opcode::HLT }),
                    operand1: None,
                    operand2: None,
                    operand3: None,
                    label: None,
                    directive: None
                }
            ))
        );
    }
}
