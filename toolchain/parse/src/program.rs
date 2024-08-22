use super::{matcher as m, module, Result};
use combine::{eof, parser::char::spaces, stream::position::Stream, EasyParser, Parser};
use lang::ast;

fn program<T>() -> impl Parser<T, Output = ast::raw::Program>
where
    T: combine::Stream<Token = char>,
    T::Position: m::Position,
{
    spaces()
        .with(module::module())
        .map(ast::meta::Program)
        .skip(eof())
}

pub fn parse(input: &str) -> Result<ast::raw::Program> {
    program().easy_parse(Stream::new(input))
}
