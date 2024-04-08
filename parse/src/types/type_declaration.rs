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

#[cfg(test)]
mod tests {
    use combine::{stream::position::Stream, EasyParser};
    use kore::{assert_eq_sorted, str};
    use lang::{ast, Range};

    fn parse(s: &str) -> crate::Result<ast::raw::TypeDeclaration> {
        super::type_declaration().easy_parse(Stream::new(s))
    }

    #[test]
    fn nil() {
        assert_eq_sorted!(
            parse("type foo = nil").unwrap().0,
            ast::raw::TypeDeclaration::raw(
                ast::TypeDeclaration::type_alias(
                    ast::raw::Binding::new(ast::Binding(str!("foo")), Range::new((1, 6), (1, 8))),
                    ast::raw::TypeExpression::raw(
                        ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                        Range::new((1, 12), (1, 14))
                    )
                ),
                Range::new((1, 1), (1, 14))
            )
        );
    }

    #[test]
    fn view() {
        assert_eq_sorted!(
            parse("view Foo (nil, boolean)").unwrap().0,
            ast::raw::TypeDeclaration::raw(
                ast::TypeDeclaration::view(
                    ast::raw::Binding::new(ast::Binding(str!("Foo")), Range::new((1, 6), (1, 8))),
                    vec![
                        ast::raw::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                            Range::new((1, 11), (1, 13))
                        ),
                        ast::raw::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                            Range::new((1, 16), (1, 22))
                        )
                    ]
                ),
                Range::new((1, 1), (1, 23))
            )
        );
    }
}
