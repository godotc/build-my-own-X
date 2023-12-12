use crate::parser::operator_parser::addition_operator;
use crate::parser::operator_parser::substraction_operator;
use crate::parser::term_parser::term;

use super::Token;
use nom::types::CompleteStr;

named!(pub expression<CompleteStr, Token>,
    ws!(
        do_parse!(
            left: term>>
            right: many0!(
                tuple!(
                    alt!(
                        addition_operator |
                        substraction_operator
                    ),
                    term
                )
            )>>
            (
                Token::Expression {
                    left: Box::new( left),
                    right: right,
                }
            )
        )
    )
);

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_expression() {
        let result = expression(CompleteStr("3*4"));
        assert_eq!(result.is_ok(), true);
    }

    #[test]
    fn test_parse_nested_expression() {
        let result = expression(CompleteStr("(3*4)+1"));
        assert_eq!(result.is_ok(), true);
    }
}
