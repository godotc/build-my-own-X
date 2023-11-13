use crate::instruction::Opcode;
use nom::types::CompleteStr;

use super::Token;

named!(pub opcode_load < CompleteStr, Token> ,
    do_parse!(
        tag!("load") >> (Token::Op { code: Opcode::LOAD })
    )
);

mod tests {

    use super::*;

    #[test]
    fn test_opcode_load() {
        let result = opcode_load(CompleteStr("load"));
        assert_eq!(result.is_ok(), true);

        let (rest, token) = result.unwrap();
        assert_eq!(rest, CompleteStr(""));
        assert_eq!(token, Token::Op { code: Opcode::LOAD });

        let result = opcode_load(CompleteStr("aload"));
        assert_eq!(result.is_ok(), false);
    }
}
