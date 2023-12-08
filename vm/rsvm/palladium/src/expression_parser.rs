use nom::types::CompleteStr;

use crate::{operand_parser::operand, operator_parser::operator, token::Token};

named!(pub expression<CompleteStr, Token>,
    ws!(
        do_parse!(
            left: operand>>
            op : operator>>
            right: operand>>
            (
                Token::Expression {
                     left: Box::new( left),
                      op: Box::new(op),
                      right: Box::new( right)
                }
            )
        )
    )
);
