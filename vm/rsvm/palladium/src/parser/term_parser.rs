use nom::types::CompleteStr;

use super::factor_parser::factor;
use super::operator_parser::division_operator;
use super::operator_parser::multiplication_operator;
use super::Token;

named!(pub term<CompleteStr, Token>,
    do_parse!(
        left: factor >>
        right: many0!(
            tuple!(
                alt!(
                    multiplication_operator|
                    division_operator
                ),
                factor
            )
        )>>
        (
            {
                Token::Term { left: Box::new(left), right: right }
            }
        )
    )
);

mod tests {
    use super::*;

    #[test]
    fn test_parse_term() {
        let result = term(CompleteStr("3*4"));
        assert_eq!(result.is_ok(), true);
    }

    #[test]
    fn test_parse_nested_term() {
        let result = term(CompleteStr("(3*4)*2"));
        assert_eq!(result.is_ok(), true);
    }

    #[test]
    fn test_parse_really_nested_term() {
        let result = term(CompleteStr("((3*4)*2)"));
        assert_eq!(result.is_ok(), true);
    }
}
