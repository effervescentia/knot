use super::{matcher as m, types::type_module, Result};
use combine::{eof, parser::char::spaces, stream::position::Stream, EasyParser, Parser};
use lang::ast;

fn typings<T>() -> impl Parser<T, Output = ast::raw::Typings>
where
    T: combine::Stream<Token = char>,
    T::Position: m::Position,
{
    spaces()
        .with(type_module::type_module())
        .map(ast::meta::Typings)
        .skip(eof())
}

pub fn parse(input: &str) -> Result<ast::raw::Typings> {
    typings().easy_parse(Stream::new(input))
}
