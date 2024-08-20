mod component;
mod declaration;
mod expression;
mod matcher;
mod module;
pub mod program;
mod statement;
#[cfg(feature = "test")]
pub mod test;
mod types;
pub mod typings;

use combine::{
    easy::Errors,
    stream::position::{SourcePosition, Stream},
};

pub type Result<'a, T> = std::result::Result<
    (T, Stream<&'a str, SourcePosition>),
    Errors<char, &'a str, SourcePosition>,
>;
