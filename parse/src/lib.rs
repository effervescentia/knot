pub mod declaration;
pub mod expression;
pub mod ksx;
pub mod matcher;
pub mod module;
pub mod statement;
pub mod types;

use combine::{
    easy::Errors,
    eof,
    parser::char::spaces,
    stream::position::{SourcePosition, Stream},
    EasyParser, Parser,
};
use knot_language::{ast::ModuleNode, Position, Program};

pub type Result<'a, T> = std::result::Result<
    (T, Stream<&'a str, SourcePosition>),
    Errors<char, &'a str, SourcePosition>,
>;

fn parse_stream<'a, T>() -> impl Parser<T, Output = ModuleNode<()>>
where
    T: combine::Stream<Token = char>,
    T::Position: Position,
{
    spaces().with(module::module()).skip(eof())
}

pub fn parse<'a>(input: &'a str) -> Result<'a, Program<()>> {
    parse_stream().map(Program).easy_parse(Stream::new(input))
}
