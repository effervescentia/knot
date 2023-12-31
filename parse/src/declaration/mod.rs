mod constant;
mod enumerated;
mod function;
mod module;
pub mod parameter;
pub mod storage;
mod type_alias;
mod view;

use crate::{common::range::Range, Position};
use combine::{choice, parser, Stream};
use lang::ast::DeclarationNode;

parser! {
    pub fn declaration[T]()(T) -> DeclarationNode<Range, ()>
    where
        [T: Stream<Token = char>, T::Position: Position]
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
    use crate::{test::fixture as f, Range};
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use kore::str;
    use lang::ast::{DeclarationNode, Expression, Module, Primitive, TypeExpression};

    fn parse(s: &str) -> crate::Result<DeclarationNode<Range, ()>> {
        super::declaration().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn type_alias() {
        assert_eq!(
            parse("type foo = nil;").unwrap().0,
            f::n::dr(
                f::a::type_("foo", f::n::txr(TypeExpression::Nil, ((1, 12), (1, 14)))),
                ((1, 1), (1, 14))
            )
        );
    }

    #[test]
    fn constant() {
        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            f::n::dr(
                f::a::const_(
                    "foo",
                    None,
                    f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 13), (1, 15)))
                ),
                ((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn enumerated() {
        assert_eq!(
            parse("enum foo = | Fizz(nil);").unwrap().0,
            f::n::dr(
                f::a::enum_(
                    "foo",
                    vec![(
                        str!("Fizz"),
                        vec![f::n::txr(TypeExpression::Nil, ((1, 19), (1, 21)))]
                    )]
                ),
                ((1, 1), (1, 22))
            )
        );
    }

    #[test]
    fn enumerated_empty_parameters() {
        assert_eq!(
            parse("enum foo = | Fizz();").unwrap().0,
            f::n::dr(
                f::a::enum_("foo", vec![(str!("Fizz"), vec![])]),
                ((1, 1), (1, 19))
            )
        );
    }

    #[test]
    fn enumerated_no_parameters() {
        assert_eq!(
            parse("enum foo = | Fizz;").unwrap().0,
            f::n::dr(
                f::a::enum_("foo", vec![(str!("Fizz"), vec![])]),
                ((1, 1), (1, 17))
            )
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            parse("func foo -> nil;").unwrap().0,
            f::n::dr(
                f::a::func_(
                    "foo",
                    vec![],
                    None,
                    f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 13), (1, 15)))
                ),
                ((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn function_result_typedef() {
        assert_eq!(
            parse("func foo: nil -> nil;").unwrap().0,
            f::n::dr(
                f::a::func_(
                    "foo",
                    vec![],
                    Some(f::n::txr(TypeExpression::Nil, ((1, 11), (1, 13)))),
                    f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 18), (1, 20)))
                ),
                ((1, 1), (1, 20))
            )
        );
    }

    #[test]
    fn function_empty_parameters() {
        assert_eq!(
            parse("func foo() -> nil;").unwrap().0,
            f::n::dr(
                f::a::func_(
                    "foo",
                    vec![],
                    None,
                    f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 15), (1, 17)))
                ),
                ((1, 1), (1, 17))
            )
        );
    }

    #[test]
    fn function_empty_parameters_result_typedef() {
        assert_eq!(
            parse("func foo(): nil -> nil;").unwrap().0,
            f::n::dr(
                f::a::func_(
                    "foo",
                    vec![],
                    Some(f::n::txr(TypeExpression::Nil, ((1, 13), (1, 15)))),
                    f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 20), (1, 22)))
                ),
                ((1, 1), (1, 22))
            )
        );
    }

    #[test]
    fn view() {
        assert_eq!(
            parse("view foo -> nil;").unwrap().0,
            f::n::dr(
                f::a::view(
                    "foo",
                    vec![],
                    f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 13), (1, 15)))
                ),
                ((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn view_empty_arguments() {
        assert_eq!(
            parse("view foo() -> nil;").unwrap().0,
            f::n::dr(
                f::a::view(
                    "foo",
                    vec![],
                    f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 15), (1, 17)))
                ),
                ((1, 1), (1, 17))
            )
        );
    }

    #[test]
    fn module_empty() {
        assert_eq!(
            parse("module foo {}").unwrap().0,
            f::n::dr(
                f::a::module("foo", f::n::mr(Module::new(vec![], vec![]))),
                ((1, 1), (1, 13))
            )
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            parse("module foo { const bar = nil; }").unwrap().0,
            f::n::dr(
                f::a::module(
                    "foo",
                    f::n::mr(Module::new(
                        vec![],
                        vec![f::n::dr(
                            f::a::const_(
                                "bar",
                                None,
                                f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 26), (1, 28)))
                            ),
                            ((1, 14), (1, 28))
                        )]
                    ))
                ),
                ((1, 1), (1, 31))
            )
        );
    }
}
