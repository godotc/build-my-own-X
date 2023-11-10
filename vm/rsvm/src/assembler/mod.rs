use crate::instruction::Opcode;

mod opcode;

pub mod opcode_parsers;
pub mod operand_parsers;
pub mod register_parsers;

#[derive(PartialEq, Debug)]
pub enum Token {
    Op { code: Opcode },
    Register { reg_num: u8 },
    IntegerOperand { value: i32 },
}
