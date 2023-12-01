use nom::opt;
use nom::types::CompleteStr;

use super::label_parser::label_declaration;
use super::opcode_parsers::opcode;
use super::operand_parsers::operand;

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
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut ret = vec![];
        match self.opcode {
            Some(Token::Op { code }) => match code {
                _ => {
                    ret.push(Into::<u8>::into(code));
                }
            },
            _ => {
                println!("Non-opcode found in opcode field! ");
                std::process::exit(1);
            }
        }

        for operand in &[&self.operand1, &self.operand2, &self.operand3] {
            if let Some(token) = operand {
                AssemblerInstruction::extract_operand(token, &mut ret);
            }
        }
        while ret.len() < 4 {
            ret.push(0)
        }

        ret
    }

    fn extract_operand(token: &Token, ret: &mut Vec<u8>) {
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
