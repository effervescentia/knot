mod constant;
mod enumerated;
mod function;
mod module;
pub mod parameter;
pub mod storage;
mod type_alias;
mod view;

use crate::{ast, matcher as m};
use combine::{choice, parser, Stream};

parser! {
    pub fn declaration[T]()(T) -> ast::raw::Declaration
    where
        [T: Stream<Token = char>, T::Position: m::Position]
    {
        choice((
            type_alias::type_alias(),
            constant::constant(),
            enumerated::enumerated(),
            function::function(),
            view::view(),
            module::module(),
        ))
    }
}

#[cfg(test)]
mod tests {
    use crate::ast;
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use kore::str;
    use lang::Range;

    fn parse(s: &str) -> crate::Result<ast::raw::Declaration> {
        super::declaration().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn type_alias() {
        assert_eq!(
            parse("type foo = nil;").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::type_alias(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 7), (1, 9))
                    )),
                    ast::raw::TypeExpression::new(
                        ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                        Range::new((1, 12), (1, 14))
                    )
                ),
                Range::new((1, 1), (1, 14))
            )
        );
    }

    #[test]
    fn constant() {
        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::constant(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 8), (1, 10))
                    )),
                    None,
                    ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 13), (1, 15))
                    )
                ),
                Range::new((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn enumerated() {
        assert_eq!(
            parse("enum foo = | Fizz(nil);").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::enumerated(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 7), (1, 9))
                    )),
                    vec![(
                        str!("Fizz"),
                        vec![ast::raw::TypeExpression::new(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                            Range::new((1, 19), (1, 21))
                        )]
                    )]
                ),
                Range::new((1, 1), (1, 22))
            )
        );
    }

    #[test]
    fn enumerated_empty_parameters() {
        assert_eq!(
            parse("enum foo = | Fizz();").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::enumerated(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 7), (1, 9))
                    )),
                    vec![(str!("Fizz"), vec![])]
                ),
                Range::new((1, 1), (1, 19))
            )
        );
    }

    #[test]
    fn enumerated_no_parameters() {
        assert_eq!(
            parse("enum foo = | Fizz;").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::enumerated(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 7), (1, 9))
                    )),
                    vec![(str!("Fizz"), vec![])]
                ),
                Range::new((1, 1), (1, 17))
            )
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            parse("func foo -> nil;").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::function(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 7), (1, 9))
                    )),
                    vec![],
                    None,
                    ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 13), (1, 15))
                    )
                ),
                Range::new((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn function_result_typedef() {
        assert_eq!(
            parse("func foo: nil -> nil;").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::function(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 7), (1, 9))
                    )),
                    vec![],
                    Some(ast::raw::TypeExpression::new(
                        ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                        Range::new((1, 11), (1, 13))
                    )),
                    ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 18), (1, 20))
                    )
                ),
                Range::new((1, 1), (1, 20))
            )
        );
    }

    #[test]
    fn function_empty_parameters() {
        assert_eq!(
            parse("func foo() -> nil;").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::function(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 7), (1, 9))
                    )),
                    vec![],
                    None,
                    ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 15), (1, 17))
                    )
                ),
                Range::new((1, 1), (1, 17))
            )
        );
    }

    #[test]
    fn function_empty_parameters_result_typedef() {
        assert_eq!(
            parse("func foo(): nil -> nil;").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::function(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 7), (1, 9))
                    )),
                    vec![],
                    Some(ast::raw::TypeExpression::new(
                        ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                        Range::new((1, 13), (1, 15))
                    )),
                    ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 20), (1, 22))
                    )
                ),
                Range::new((1, 1), (1, 22))
            )
        );
    }

    #[test]
    fn view() {
        assert_eq!(
            parse("view foo -> nil;").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::view(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 6), (1, 8))
                    )),
                    vec![],
                    ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 13), (1, 15))
                    )
                ),
                Range::new((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn view_empty_arguments() {
        assert_eq!(
            parse("view foo() -> nil;").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::view(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 6), (1, 8))
                    )),
                    vec![],
                    ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 15), (1, 17))
                    )
                ),
                Range::new((1, 1), (1, 17))
            )
        );
    }

    #[test]
    fn module_empty() {
        assert_eq!(
            parse("module foo {}").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::module(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 8), (1, 10))
                    )),
                    ast::raw::Module::new(
                        ast::Module::new(vec![], vec![]),
                        Range::new((1, 1), (1, 13))
                    )
                ),
                Range::new((1, 1), (1, 13))
            )
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            parse("module foo { const bar = nil; }").unwrap().0,
            ast::raw::Declaration::new(
                ast::Declaration::module(
                    ast::Storage::public(ast::raw::Binding::new(
                        ast::Binding(str!("foo")),
                        Range::new((1, 8), (1, 10))
                    )),
                    ast::raw::Module::new(
                        ast::Module::new(
                            vec![],
                            vec![ast::raw::Declaration::new(
                                ast::Declaration::constant(
                                    ast::Storage::public(ast::raw::Binding::new(
                                        ast::Binding(str!("bar")),
                                        Range::new((1, 20), (1, 23))
                                    )),
                                    None,
                                    ast::raw::Expression::new(
                                        ast::Expression::Primitive(ast::Primitive::Nil),
                                        Range::new((1, 26), (1, 28))
                                    )
                                ),
                                Range::new((1, 14), (1, 28))
                            )]
                        ),
                        Range::new((1, 1), (1, 31))
                    )
                ),
                Range::new((1, 1), (1, 31))
            )
        );
    }
}
