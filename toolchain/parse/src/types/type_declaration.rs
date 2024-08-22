use crate::matcher as m;
use combine::{choice, optional, Parser, Stream};
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
        super::type_expression::type_expression(),
    ))
    .map(|((_, start), binding, attributes)| {
        let range = &start + attributes.0.range();

        ast::raw::TypeDeclaration::raw(ast::TypeDeclaration::view(binding, attributes), range)
    })
}

fn function<T>() -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::terminated((
        m::keyword("func"),
        m::binding(),
        m::lambda(
            optional(m::tuple(super::type_expression::type_expression())),
            super::type_expression::type_expression(),
        ),
    ))
    .map(|((_, start), binding, (parameters, result))| {
        let range = &start + result.0.range();

        ast::raw::TypeDeclaration::raw(
            ast::TypeDeclaration::function(
                binding,
                parameters.map(|x| x.0).unwrap_or_default(),
                result,
            ),
            range,
        )
    })
}

fn module<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeModule>,
{
    m::terminated((m::keyword("module"), m::binding(), m::closure(parser))).map(
        |((_, start), binding, (module, end))| {
            let range = &start + &end;

            ast::raw::TypeDeclaration::raw(ast::TypeDeclaration::module(binding, module), range)
        },
    )
}

pub fn type_declaration<T, P, F>(parser: F) -> impl Parser<T, Output = ast::raw::TypeDeclaration>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeModule>,
    F: Fn() -> P,
{
    choice((type_alias(), view(), function(), module(parser())))
}

#[cfg(test)]
mod tests {
    use combine::{stream::position::Stream, EasyParser};
    use kore::{assert_eq, str};
    use lang::{ast, Range};

    fn parse(s: &str) -> crate::Result<ast::raw::TypeDeclaration> {
        super::type_declaration(crate::types::type_module::type_module).easy_parse(Stream::new(s))
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
            parse("view Foo { bar: nil, fizz?: boolean }").unwrap().0,
            ast::raw::TypeDeclaration::raw(
                ast::TypeDeclaration::view(
                    ast::raw::Binding::new(ast::Binding(str!("Foo")), Range::new((1, 6), (1, 8))),
                    ast::raw::TypeExpression::raw(
                        ast::TypeExpression::Object(vec![
                            ast::ObjectTypeExpressionEntry::Required(
                                ast::raw::Binding::new(
                                    ast::Binding(str!("bar")),
                                    Range::new((1, 12), (1, 14))
                                ),
                                ast::raw::TypeExpression::raw(
                                    ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                                    Range::new((1, 17), (1, 19))
                                )
                            ),
                            ast::ObjectTypeExpressionEntry::Optional(
                                ast::raw::Binding::new(
                                    ast::Binding(str!("fizz")),
                                    Range::new((1, 22), (1, 25))
                                ),
                                ast::raw::TypeExpression::raw(
                                    ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                                    Range::new((1, 29), (1, 35))
                                )
                            )
                        ]),
                        Range::new((1, 10), (1, 37))
                    )
                ),
                Range::new((1, 1), (1, 37))
            )
        );
    }

    #[test]
    fn function_no_parameters() {
        assert_eq!(
            parse("func foo -> string;").unwrap().0,
            ast::raw::TypeDeclaration::raw(
                ast::TypeDeclaration::function(
                    ast::raw::Binding::new(ast::Binding(str!("foo")), Range::new((1, 6), (1, 8))),
                    vec![],
                    ast::raw::TypeExpression::raw(
                        ast::TypeExpression::Primitive(ast::TypePrimitive::String),
                        Range::new((1, 13), (1, 18))
                    )
                ),
                Range::new((1, 1), (1, 18))
            )
        );
    }

    #[test]
    fn function_with_parameters() {
        assert_eq!(
            parse("func foo (integer, boolean) -> string;").unwrap().0,
            ast::raw::TypeDeclaration::raw(
                ast::TypeDeclaration::function(
                    ast::raw::Binding::new(ast::Binding(str!("foo")), Range::new((1, 6), (1, 8))),
                    vec![
                        ast::raw::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Integer),
                            Range::new((1, 11), (1, 17))
                        ),
                        ast::raw::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                            Range::new((1, 20), (1, 26))
                        )
                    ],
                    ast::raw::TypeExpression::raw(
                        ast::TypeExpression::Primitive(ast::TypePrimitive::String),
                        Range::new((1, 32), (1, 37))
                    )
                ),
                Range::new((1, 1), (1, 37))
            )
        );
    }

    #[test]
    fn empty_module() {
        assert_eq!(
            parse("module foo {}").unwrap().0,
            ast::raw::TypeDeclaration::raw(
                ast::TypeDeclaration::module(
                    ast::raw::Binding::new(ast::Binding(str!("foo")), Range::new((1, 8), (1, 10))),
                    ast::raw::TypeModule::raw(
                        ast::TypeModule::new(vec![]),
                        Range::new((1, 13), (1, 13))
                    ),
                ),
                Range::new((1, 1), (1, 13))
            )
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            parse(
                "module foo {
  type Bar = nil;
}"
            )
            .unwrap()
            .0,
            ast::raw::TypeDeclaration::raw(
                ast::TypeDeclaration::module(
                    ast::raw::Binding::new(ast::Binding(str!("foo")), Range::new((1, 8), (1, 10))),
                    ast::raw::TypeModule::raw(
                        ast::TypeModule::new(vec![ast::raw::TypeDeclaration::raw(
                            ast::TypeDeclaration::type_alias(
                                ast::raw::Binding::new(
                                    ast::Binding(str!("Bar")),
                                    Range::new((2, 8), (2, 10))
                                ),
                                ast::raw::TypeExpression::raw(
                                    ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                                    Range::new((2, 14), (2, 16))
                                )
                            ),
                            Range::new((2, 3), (2, 16))
                        )]),
                        Range::new((2, 3), (3, 0))
                    ),
                ),
                Range::new((1, 1), (3, 1))
            )
        );
    }
}
