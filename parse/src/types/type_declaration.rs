use crate::matcher as m;
use combine::{choice, Parser, Stream};
use lang::ast;

fn type_alias<T>() -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        m::keyword("type"),
        m::binding(),
        m::symbol('='),
        super::type_expression::type_expression(),
    ))
    .map(|((_, start), binding, _, value)| {
        let end = value.0.range();
        let range = &start + end;

        ast::raw::TypeDeclaration::raw(ast::TypeDeclaration::type_alias(binding, value), range)
    })
}

fn view<T>() -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        m::keyword("view"),
        m::binding(),
        m::tuple(super::type_expression::type_expression()),
    ))
    .map(|((_, start), binding, (parameters, end))| {
        let range = &start + &end;

        ast::raw::TypeDeclaration::raw(ast::TypeDeclaration::view(binding, parameters), range)
    })
}

pub fn type_declaration<T>() -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    choice((type_alias(), view()))
}
