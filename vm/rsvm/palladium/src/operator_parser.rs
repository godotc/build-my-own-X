use nom::types::CompleteStr;

use crate::token::Token;

named!(pub operator<CompleteStr, Token>,
    ws!(
        alt!(
            addition_operator|
            subtraction_operator|
            multiplication_operator|
            division_operator
        )
    )
);

named!(addition_operator<CompleteStr, Token>,
    ws!(
        do_parse!(
            tag!("+") >>
            (
                Token::AdditionOperator
            )
        )
    )
);

named!(subtraction_operator<CompleteStr, Token>,
    ws!(
        do_parse!(
            tag!("-") >>
            (
                Token::AdditionOperator
            )
        )
    )
);

named!(multiplication_operator<CompleteStr, Token>,
    ws!(
        do_parse!(
            tag!("*") >>
            (
                Token::AdditionOperator
            )
        )
    )
);

named!(division_operator<CompleteStr, Token>,
    ws!(
        do_parse!(
            tag!("/") >>
            (
                Token::AdditionOperator
            )
        )
    )
);
