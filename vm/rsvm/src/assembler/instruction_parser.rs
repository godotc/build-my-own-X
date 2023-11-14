use nom::types::CompleteStr;

use super::opcode_parsers::opcode_load;
use super::operand_parsers::integer_operand;
use super::register_parsers::register;
use super::Token;

#[derive(Debug, PartialEq)]
pub struct AssemblerInstruction {
    label: Option<String>,
    opcode: Token,
    operand1: Option<Token>,
    operand2: Option<Token>,
    operand3: Option<Token>,
}

// I don't know why this, but it did compile succuess

/// Handles instructions of the following form:
/// LOAD $0 #100
named!(pub instruction_one<CompleteStr, AssemblerInstruction >,
    do_parse!(
        o: opcode_load >>
        r: register >>
        i: integer_operand >>
        (
            AssemblerInstruction{
                label: None,
                opcode: o,
                operand1: Some(r),
                operand2: Some(i),
                operand3:None ,
            }
        )
    )
);

impl AssemblerInstruction {
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut ret = vec![];
        match self.opcode {
            Token::Op { code } => match code {
                _ => {
                    ret.push(code as u8);
                }
            },
            _ => {
                println!("Non-opcode found in opcode field! ");
                std::process::exit(1);
            }
        }

        for operand in vec![&self.operand1, &self.operand2, &self.operand3] {
            match operand {
                Some(t) => AssemblerInstruction::extract_operant(t, &mut ret),
                None => {}
            }
        }
        ret
    }

    pub fn extract_operant(token: &Token, ret: &mut Vec<u8>) {
        match token {
            Token::Register { reg_num } => {
                ret.push(*reg_num);
            }
            Token::IntegerOperand { value } => {
                let raw = *value as u16;
                let byte1 = raw; // low
                let byte2 = raw >> 8; // high
                ret.push(byte2 as u8);
                ret.push(byte1 as u8);
            }
            _ => {
                println!("Opcode found in operand field");
                std::process::exit(1);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::instruction::Opcode;

    use super::*;

    #[test]
    fn test_parse_instruction_form_one() {
        let result = instruction_one(CompleteStr("load $0 #100\n"));
        assert_eq!(
            result,
            Ok((
                CompleteStr(""),
                AssemblerInstruction {
                    label: None,
                    opcode: Token::Op { code: Opcode::LOAD },
                    operand1: Some(Token::Register { reg_num: 0 }),
                    operand2: Some(Token::IntegerOperand { value: 100 }),
                    operand3: None
                }
            ))
        );
    }
}
