use nom::{digit, opt, tag, types::CompleteStr};

use super::Token;

// named!(pub operand<CompleteStr, Token>,
//     ws!(
//         alt!(
// integer
//         )
//     )
// );
