pub mod declaration;
pub mod expression;
pub mod ksx;
pub mod matcher;
pub mod module;
pub mod statement;
pub mod types;

use crate::{ast::ModuleNode, common::position::Position};
use combine::{
    easy::{self, Errors},
    eof,
    parser::char::spaces,
    stream::position::{SourcePosition, Stream},
    EasyParser, Parser,
};
use std::fmt::Debug;

type PositionStream<'a> = Stream<&'a str, SourcePosition>;
pub type CharStream<'a> = easy::Stream<PositionStream<'a>>;

pub type ParseResult<'a, T> =
    Result<(T, PositionStream<'a>), Errors<char, &'a str, SourcePosition>>;

#[derive(Debug, PartialEq)]
pub struct Program<C>(pub ModuleNode<C>);

fn parse_stream<'a, T>() -> impl Parser<T, Output = ModuleNode<()>>
where
    T: combine::Stream<Token = char>,
    T::Position: Position,
{
    spaces().with(module::module()).skip(eof())
}

pub fn parse<'a>(input: &'a str) -> ParseResult<'a, Program<()>> {
    parse_stream().map(Program).easy_parse(Stream::new(input))
}
