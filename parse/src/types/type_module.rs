use super::type_declaration;
use crate::matcher as m;
use combine::{many, Parser, Stream};
use lang::ast;

pub fn type_module<T>() -> impl Parser<T, Output = ast::raw::TypeModule>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::span(many::<Vec<_>, _, _>(type_declaration::type_declaration())).map(
        |(declarations, range)| ast::meta::TypeModule::raw(ast::TypeModule { declarations }, range),
    )
}
