pub mod declaration;
pub mod expression;
pub mod ksx;
pub mod matcher;
pub mod module;
pub mod node;
pub mod position;
pub mod range;
pub mod statement;
pub mod types;
use combine::{
    easy::{self, Errors},
    stream::position::{SourcePosition, Stream},
};
extern crate combine;

pub type CharStream<'a> = easy::Stream<Stream<&'a str, SourcePosition>>;

pub type ParseResult<'a, T> =
    Result<(T, Stream<&'a str, SourcePosition>), Errors<char, &'a str, SourcePosition>>;
