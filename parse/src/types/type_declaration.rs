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
        m::between(
            m::symbol('('),
            m::symbol(')'),
            super::type_expression::type_expression(),
        ),
    ))
    .map(|((_, start), binding, (attributes, end))| {
        let range = &start + &end;

        ast::raw::TypeDeclaration::raw(ast::TypeDeclaration::view(binding, attributes), range)
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
    use kore::{assert_eq, str};
    use lang::{ast, Range};

    fn parse(s: &str) -> crate::Result<ast::raw::TypeDeclaration> {
        super::type_declaration().easy_parse(Stream::new(s))
    }

    #[test]
    fn nil() {
        assert_eq!(
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
        assert_eq!(
            parse("view Foo ({ bar: nil, fizz?: boolean })").unwrap().0,
            ast::raw::TypeDeclaration::raw(
                ast::TypeDeclaration::view(
                    ast::raw::Binding::new(ast::Binding(str!("Foo")), Range::new((1, 6), (1, 8))),
                    ast::raw::TypeExpression::raw(
                        ast::TypeExpression::Object(vec![
                            ast::ObjectTypeExpressionEntry::Required(
                                ast::raw::Binding::new(
                                    ast::Binding(str!("bar")),
                                    Range::new((1, 13), (1, 15))
                                ),
                                ast::raw::TypeExpression::raw(
                                    ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                                    Range::new((1, 18), (1, 20))
                                )
                            ),
                            ast::ObjectTypeExpressionEntry::Optional(
                                ast::raw::Binding::new(
                                    ast::Binding(str!("fizz")),
                                    Range::new((1, 23), (1, 26))
                                ),
                                ast::raw::TypeExpression::raw(
                                    ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                                    Range::new((1, 30), (1, 36))
                                )
                            )
                        ]),
                        Range::new((1, 11), (1, 38))
                    )
                ),
                Range::new((1, 1), (1, 39))
            )
        );
    }
}
