mod common;
mod declaration;
mod expression;
mod ksx;
mod matcher;
mod module;
mod statement;
#[cfg(feature = "test")]
pub mod test;
mod types;

use combine::{
    easy::Errors,
    eof,
    parser::char::spaces,
    stream::position::{SourcePosition, Stream},
    EasyParser, Parser,
};
pub use common::{position::Position, range::Range};
use lang::{ast::ModuleNode, Program};

pub type Result<'a, T> = std::result::Result<
    (T, Stream<&'a str, SourcePosition>),
    Errors<char, &'a str, SourcePosition>,
>;

fn program<T>() -> impl Parser<T, Output = ModuleNode<Range, ()>>
where
    T: combine::Stream<Token = char>,
    T::Position: Position,
{
    spaces().with(module::module()).skip(eof())
}

pub fn parse(input: &str) -> Result<Program<Range, ()>> {
    program().map(Program).easy_parse(Stream::new(input))
}
