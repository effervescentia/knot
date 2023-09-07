mod constant;
mod enumerated;
mod function;
mod module;
pub mod parameter;
pub mod storage;
mod type_alias;
mod view;
use crate::{
    expression::ExpressionRaw, module::ModuleRaw, position::Decrement, range::Range,
    types::type_expression::TypeExpression,
};
use combine::{choice, parser, Parser, Stream};
use parameter::Parameter;
use std::fmt::Debug;
use storage::Storage;

#[derive(Debug, PartialEq)]
pub enum Declaration<E, M> {
    TypeAlias {
        name: Storage,
        value: TypeExpression,
    },
    Constant {
        name: Storage,
        value_type: Option<TypeExpression>,
        value: E,
    },
    Enumerated {
        name: Storage,
        variants: Vec<(String, Vec<TypeExpression>)>,
    },
    Function {
        name: Storage,
        parameters: Vec<Parameter<E>>,
        body_type: Option<TypeExpression>,
        body: E,
    },
    View {
        name: Storage,
        parameters: Vec<Parameter<E>>,
        body: E,
    },
    Module {
        name: Storage,
        value: M,
    },
}

#[derive(Debug, PartialEq)]
pub struct DeclarationRaw<T>(
    pub Declaration<ExpressionRaw<T>, ModuleRaw<T>>,
    pub Range<T>,
)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement + Decrement;

impl<T> DeclarationRaw<T>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn get_range(&self) -> &Range<T> {
        match self {
            DeclarationRaw(_, range) => range,
        }
    }
}

fn declaration_<T>() -> impl Parser<T, Output = DeclarationRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
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

parser! {
    pub fn declaration[T]()(T) -> DeclarationRaw<T>
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug + Decrement]
    {
        declaration_()
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        declaration::{declaration, storage::Visibility, Declaration, DeclarationRaw, Storage},
        expression::{primitive::Primitive, Expression, ExpressionRaw},
        module::{Module, ModuleRaw},
        range::Range,
        types::type_expression::TypeExpression,
        CharStream, ParseResult,
    };
    use combine::{stream::position::Stream, EasyParser};

    fn parse(s: &str) -> ParseResult<DeclarationRaw<CharStream>> {
        declaration().easy_parse(Stream::new(s))
    }

    #[test]
    fn type_alias() {
        assert_eq!(
            parse("type foo = nil;").unwrap().0,
            DeclarationRaw(
                Declaration::TypeAlias {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value: TypeExpression::Nil
                },
                Range::chars((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn constant() {
        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            DeclarationRaw(
                Declaration::Constant {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value_type: None,
                    value: ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::chars((1, 13), (1, 15))
                    )
                },
                Range::chars((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn enumerated() {
        assert_eq!(
            parse("enum foo = | Fizz | Buzz(nil);").unwrap().0,
            DeclarationRaw(
                Declaration::Enumerated {
                    name: Storage(Visibility::Public, String::from("foo")),
                    variants: vec![
                        (String::from("Fizz"), vec![]),
                        (String::from("Buzz"), vec![TypeExpression::Nil])
                    ]
                },
                Range::chars((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            parse("func foo -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: None,
                    body: ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::chars((1, 1), (1, 15))
                    )
                },
                Range::chars((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn function_result_typedef() {
        assert_eq!(
            parse("func foo: nil -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: Some(TypeExpression::Nil),
                    body: ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::chars((1, 1), (1, 15))
                    )
                },
                Range::chars((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn function_empty_parameters() {
        assert_eq!(
            parse("func foo() -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: None,
                    body: ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::chars((1, 1), (1, 15))
                    )
                },
                Range::chars((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn function_empty_parameters_result_typedef() {
        assert_eq!(
            parse("func foo(): nil -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: Some(TypeExpression::Nil),
                    body: ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::chars((1, 1), (1, 15))
                    )
                },
                Range::chars((1, 1), (1, 15))
            )
        );
    }

    #[test]
    fn view() {
        assert_eq!(
            parse("view foo -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::View {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body: ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::chars((1, 1), (1, 1))
                    )
                },
                Range::chars((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn view_empty_arguments() {
        assert_eq!(
            parse("view foo() -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::View {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body: ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::chars((1, 1), (1, 1))
                    )
                },
                Range::chars((1, 1), (1, 1))
            )
        );
    }

    #[test]
    fn module_empty() {
        assert_eq!(
            parse("module foo {}").unwrap().0,
            DeclarationRaw(
                Declaration::Module {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value: ModuleRaw(Module::new(vec![], vec![]))
                },
                Range::chars((1, 1), (1, 13))
            )
        );
    }

    #[test]
    fn module() {
        assert_eq!(
            parse("module foo { const bar = nil; }").unwrap().0,
            DeclarationRaw(
                Declaration::Module {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value: ModuleRaw(Module::new(
                        vec![],
                        vec![DeclarationRaw(
                            Declaration::Constant {
                                name: Storage(Visibility::Public, String::from("bar")),
                                value_type: None,
                                value: ExpressionRaw(
                                    Expression::Primitive(Primitive::Nil),
                                    Range::chars((1, 26), (1, 28))
                                )
                            },
                            Range::chars((1, 14), (1, 28))
                        )]
                    ),)
                },
                Range::chars((1, 1), (1, 31))
            )
        );
    }
}
