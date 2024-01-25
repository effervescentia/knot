pub mod ast;
mod component;
mod declaration;
mod expression;
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
use matcher as m;

pub type Result<'a, T> = std::result::Result<
    (T, Stream<&'a str, SourcePosition>),
    Errors<char, &'a str, SourcePosition>,
>;

fn program<T>() -> impl Parser<T, Output = ast::raw::Program>
where
    T: combine::Stream<Token = char>,
    T::Position: m::Position,
{
    spaces()
        .with(module::module())
        .map(ast::raw::Program)
        .skip(eof())
}

pub fn parse(input: &str) -> Result<ast::raw::Program> {
    program().easy_parse(Stream::new(input))
}
