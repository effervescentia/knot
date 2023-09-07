use combine::{
    easy::{self, Errors},
    stream::position::{SourcePosition, Stream},
};
extern crate combine;

pub type CharStream<'a> = easy::Stream<Stream<&'a str, SourcePosition>>;

pub type ParseResult<'a, T> =
    Result<(T, Stream<&'a str, SourcePosition>), Errors<char, &'a str, SourcePosition>>;

mod analyzer;
mod declaration;
mod expression;
mod matcher;
mod module;
mod position;
mod range;
mod test;
mod types;
