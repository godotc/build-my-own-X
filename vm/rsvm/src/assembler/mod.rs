use crate::instruction::Opcode;

pub mod instruction_parser;
pub mod opcode_parsers;
pub mod operand_parsers;
pub mod program_parser;
pub mod register_parsers;

#[derive(PartialEq, Debug)]
pub enum Token {
    Op { code: Opcode },
    Register { reg_num: u8 },
    IntegerOperand { value: i32 },
}
