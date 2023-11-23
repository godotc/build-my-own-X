use crate::instruction::Opcode;
use nom::alpha1;
use nom::types::CompleteStr;

use super::Token;

named!(pub opcode < CompleteStr, Token> ,
    do_parse!(
        opcode: alpha1 >>
        (
            Token::Op{code: Opcode::from(opcode)}
        )
    )
);

#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn test_opcode_parse() {
        let result = opcode(CompleteStr("load"));
        assert_eq!(result.is_ok(), true);

        let (rest, token) = result.unwrap();
        assert_eq!(rest, CompleteStr(""));
        assert_eq!(token, Token::Op { code: Opcode::LOAD });

        let result = opcode(CompleteStr("aload"));
        assert_eq!(result.unwrap().1, Token::Op { code: Opcode::IGL });
    }
}
