use super::type_declaration;
use crate::{matcher as m, module::import};
use combine::{many, parser, skip_many, Parser, Stream};
use lang::ast;

fn type_module_0<T>() -> impl Parser<T, Output = ast::raw::TypeModule>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    let comments = || skip_many(m::comment());

    m::span((
        comments().with(many::<Vec<_>, _, _>(import::import().skip(comments()))),
        many::<Vec<_>, _, _>(type_declaration::type_declaration(type_module).skip(comments())),
    ))
    .map(|((imports, declarations), range)| {
        ast::meta::TypeModule::raw(
            ast::TypeModule {
                imports,
                declarations,
            },
            range,
        )
    })
}

parser! {
    pub fn type_module[T]()(T) -> ast::raw::TypeModule
    where
        [T: Stream<Token = char>, T::Position: m::Position]
    {
        type_module_0()
    }
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
view Foo { bar: nil, fizz?: boolean };"
            )
            .unwrap()
            .0,
            ast::raw::TypeModule::raw(
                ast::TypeModule::new(
                    vec![],
                    vec![
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
                                ast::raw::TypeExpression::raw(
                                    ast::TypeExpression::Object(vec![
                                        ast::ObjectTypeExpressionEntry::Required(
                                            ast::raw::Binding::new(
                                                ast::Binding(str!("bar")),
                                                Range::new((2, 12), (2, 14))
                                            ),
                                            ast::raw::TypeExpression::raw(
                                                ast::TypeExpression::Primitive(
                                                    ast::TypePrimitive::Nil
                                                ),
                                                Range::new((2, 17), (2, 19))
                                            )
                                        ),
                                        ast::ObjectTypeExpressionEntry::Optional(
                                            ast::raw::Binding::new(
                                                ast::Binding(str!("fizz")),
                                                Range::new((2, 22), (2, 25))
                                            ),
                                            ast::raw::TypeExpression::raw(
                                                ast::TypeExpression::Primitive(
                                                    ast::TypePrimitive::Boolean
                                                ),
                                                Range::new((2, 29), (2, 35))
                                            )
                                        )
                                    ]),
                                    Range::new((2, 10), (2, 37))
                                )
                            ),
                            Range::new((2, 1), (2, 37))
                        )
                    ]
                ),
                Range::new((1, 1), (2, 38))
            )
        );
    }
}
