mod constant;
mod enumerated;
mod function;
mod module;
pub mod parameter;
pub mod storage;
mod type_alias;
mod view;
use crate::parser::{
    expression::ExpressionNode,
    module::ModuleNode,
    position::Decrement,
    range::{Range, Ranged},
    types::type_expression::TypeExpressionNode,
};
use combine::{choice, parser, Stream};
use parameter::Parameter;
use std::fmt::Debug;
use storage::Storage;

use super::node::Node;

#[derive(Debug, PartialEq)]
pub enum Declaration<E, M, T> {
    TypeAlias {
        name: Storage,
        value: T,
    },
    Constant {
        name: Storage,
        value_type: Option<T>,
        value: E,
    },
    Enumerated {
        name: Storage,
        variants: Vec<(String, Vec<T>)>,
    },
    Function {
        name: Storage,
        parameters: Vec<Parameter<E, T>>,
        body_type: Option<T>,
        body: E,
    },
    View {
        name: Storage,
        parameters: Vec<Parameter<E, T>>,
        body: E,
    },
    Module {
        name: Storage,
        value: M,
    },
}

type RawValue<T> = Declaration<ExpressionNode<T, ()>, ModuleNode<T, ()>, TypeExpressionNode<T, ()>>;

#[derive(Debug, PartialEq)]
pub struct DeclarationNode<T, C>(pub Node<RawValue<T>, T, C>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement;

impl<T> DeclarationNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn raw(x: RawValue<T>, range: Range<T>) -> Self {
        Self(Node::raw(x, range))
    }
}

parser! {
    pub fn declaration[T]()(T) -> DeclarationNode<T, ()>
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug + Decrement]
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
    use crate::{
        parser::{
            declaration::{
                declaration, storage::Visibility, Declaration, DeclarationNode, Storage,
            },
            expression::{primitive::Primitive, Expression},
            module::Module,
            types::type_expression::TypeExpression,
            CharStream, ParseResult,
        },
        test::fixture as f,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<DeclarationNode<CharStream, ()>> {
        declaration().easy_parse(Stream::new(s))
    }

    #[test]
    fn type_alias() {
        assert_eq!(
            parse("type foo = nil;").unwrap().0,
            f::dr(
                Declaration::TypeAlias {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value: f::txr(TypeExpression::Nil, ((1, 12), (1, 14)))
                },
                ((1, 1), (1, 14))
            )
        );
    }

    #[test]
    fn constant() {
        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            f::dr(
                Declaration::Constant {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value_type: None,
                    value: f::xr(Expression::Primitive(Primitive::Nil), ((1, 13), (1, 15)))
                },
                ((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn enumerated() {
        assert_eq!(
            parse("enum foo = | Fizz(nil);").unwrap().0,
            f::dr(
                Declaration::Enumerated {
                    name: Storage(Visibility::Public, String::from("foo")),
                    variants: vec![(
                        String::from("Fizz"),
                        vec![f::txr(TypeExpression::Nil, ((1, 19), (1, 21)))]
                    )]
                },
                ((1, 1), (1, 22))
            )
        );
    }

    #[test]
    fn enumerated_empty_parameters() {
        assert_eq!(
            parse("enum foo = | Fizz();").unwrap().0,
            f::dr(
                Declaration::Enumerated {
                    name: Storage(Visibility::Public, String::from("foo")),
                    variants: vec![(String::from("Fizz"), vec![]),]
                },
                ((1, 1), (1, 19))
            )
        );
    }

    #[test]
    fn enumerated_no_parameters() {
        assert_eq!(
            parse("enum foo = | Fizz;").unwrap().0,
            f::dr(
                Declaration::Enumerated {
                    name: Storage(Visibility::Public, String::from("foo")),
                    variants: vec![(String::from("Fizz"), vec![]),]
                },
                ((1, 1), (1, 17))
            )
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            parse("func foo -> nil;").unwrap().0,
            f::dr(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: None,
                    body: f::xr(Expression::Primitive(Primitive::Nil), ((1, 13), (1, 15)))
                },
                ((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn function_result_typedef() {
        assert_eq!(
            parse("func foo: nil -> nil;").unwrap().0,
            f::dr(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: Some(f::txr(TypeExpression::Nil, ((1, 11), (1, 13)))),
                    body: f::xr(Expression::Primitive(Primitive::Nil), ((1, 18), (1, 20)))
                },
                ((1, 1), (1, 20))
            )
        );
    }

    #[test]
    fn function_empty_parameters() {
        assert_eq!(
            parse("func foo() -> nil;").unwrap().0,
            f::dr(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: None,
                    body: f::xr(Expression::Primitive(Primitive::Nil), ((1, 15), (1, 17)))
                },
                ((1, 1), (1, 17))
            )
        );
    }

    #[test]
    fn function_empty_parameters_result_typedef() {
        assert_eq!(
            parse("func foo(): nil -> nil;").unwrap().0,
            f::dr(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: Some(f::txr(TypeExpression::Nil, ((1, 13), (1, 15)))),
                    body: f::xr(Expression::Primitive(Primitive::Nil), ((1, 20), (1, 22)))
                },
                ((1, 1), (1, 22))
            )
        );
    }

    #[test]
    fn view() {
        assert_eq!(
            parse("view foo -> nil;").unwrap().0,
            f::dr(
                Declaration::View {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body: f::xr(Expression::Primitive(Primitive::Nil), ((1, 13), (1, 15)))
                },
                ((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn view_empty_arguments() {
        assert_eq!(
            parse("view foo() -> nil;").unwrap().0,
            f::dr(
                Declaration::View {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body: f::xr(Expression::Primitive(Primitive::Nil), ((1, 15), (1, 17)))
                },
                ((1, 1), (1, 17))
            )
        );
    }

    #[test]
    fn module_empty() {
        assert_eq!(
            parse("module foo {}").unwrap().0,
            f::dr(
                Declaration::Module {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value: f::mr(Module::new(vec![], vec![]))
                },
                ((1, 1), (1, 13))
            )
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            parse("module foo { const bar = nil; }").unwrap().0,
            f::dr(
                Declaration::Module {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value: f::mr(Module::new(
                        vec![],
                        vec![f::dr(
                            Declaration::Constant {
                                name: Storage(Visibility::Public, String::from("bar")),
                                value_type: None,
                                value: f::xr(
                                    Expression::Primitive(Primitive::Nil),
                                    ((1, 26), (1, 28))
                                )
                            },
                            ((1, 14), (1, 28))
                        )]
                    ),)
                },
                ((1, 1), (1, 31))
            )
        );
    }
}
