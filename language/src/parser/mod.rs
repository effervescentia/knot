extern crate combine;

pub mod declaration;
pub mod expression;
pub mod ksx;
pub mod matcher;
pub mod module;
pub mod statement;
pub mod types;

use crate::ast::ModuleNode;
use combine::{
    easy::{self, Errors},
    eof,
    parser::char::spaces,
    stream::position::{SourcePosition, Stream},
    EasyParser, Parser,
};

pub type CharStream<'a> = easy::Stream<Stream<&'a str, SourcePosition>>;

pub type ParseResult<'a, T> =
    Result<(T, Stream<&'a str, SourcePosition>), Errors<char, &'a str, SourcePosition>>;

pub fn parse(
    input: &str,
) -> Result<
    (ModuleNode<CharStream, ()>, Stream<&str, SourcePosition>),
    Errors<char, &str, SourcePosition>,
> {
    spaces()
        .with(module::module())
        .skip(eof())
        .easy_parse(Stream::new(input))
}
