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

#[cfg(test)]
mod tests {
    use combine::{stream::position::Stream, EasyParser};
    use kore::{assert_eq_sorted, str};
    use lang::{ast, Range};

    fn parse(s: &str) -> crate::Result<ast::raw::TypeModule> {
        super::type_module().easy_parse(Stream::new(s))
    }

    #[test]
    fn module() {
        assert_eq_sorted!(
            parse(
                "type foo = nil;
view Foo (nil, boolean);"
            )
            .unwrap()
            .0,
            ast::raw::TypeModule::raw(
                ast::TypeModule::new(vec![
                    ast::raw::TypeDeclaration::raw(
                        ast::TypeDeclaration::type_alias(
                            ast::raw::Binding::new(
                                ast::Binding(str!("foo")),
                                Range::new((1, 6), (1, 8))
                            ),
                            ast::raw::TypeExpression::raw(
                                ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                                Range::new((1, 12), (1, 14))
                            )
                        ),
                        Range::new((1, 1), (1, 14))
                    ),
                    ast::raw::TypeDeclaration::raw(
                        ast::TypeDeclaration::view(
                            ast::raw::Binding::new(
                                ast::Binding(str!("Foo")),
                                Range::new((2, 6), (2, 8))
                            ),
                            vec![
                                ast::raw::TypeExpression::raw(
                                    ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                                    Range::new((2, 11), (2, 13))
                                ),
                                ast::raw::TypeExpression::raw(
                                    ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                                    Range::new((2, 16), (2, 22))
                                )
                            ]
                        ),
                        Range::new((2, 1), (2, 23))
                    )
                ]),
                Range::new((1, 1), (2, 24))
            )
        );
    }
}
