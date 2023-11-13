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

/// Handles instructions of the following form:
/// LOAD $0 #100
named!(pub instruction_one<CompleteStr, AssemblerInstruction>,
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
