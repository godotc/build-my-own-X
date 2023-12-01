use crate::instruction::Opcode;

pub mod instruction_parser;
pub mod opcode_parsers;
pub mod operand_parsers;
pub mod program_parser;
pub mod register_parsers;
pub mod directive_parser;
pub mod label_parser;

#[derive(PartialEq, Debug)]
pub enum Token {
    Op { code: Opcode },
    Register { reg_num: u8 },
    IntegerOperand { value: i32 },
    LabelDeclaration { name: String },
    LabelUsage { name: String },
    Directive { name: String },
}
