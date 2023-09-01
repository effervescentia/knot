mod constant;
mod enumerated;
mod function;
mod module;
pub mod parameter;
pub mod storage;
mod type_alias;
mod view;
use crate::{
    expression::ExpressionRaw, module::ModuleRaw, range::Range,
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
    T::Position: Copy + Debug;

impl<T> DeclarationRaw<T>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
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
    T::Position: Copy + Debug,
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
        [T: Stream<Token = char>, T::Position: Copy + Debug]
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
    };
    use combine::Parser;

    #[test]
    fn type_alias() {
        let parse = |s| declaration().parse(s);

        assert_eq!(
            parse("type foo = nil;").unwrap().0,
            DeclarationRaw(
                Declaration::TypeAlias {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value: TypeExpression::Nil
                },
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn constant() {
        let parse = |s| declaration().parse(s);

        assert_eq!(
            parse("const foo = nil;").unwrap().0,
            DeclarationRaw(
                Declaration::Constant {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value_type: None,
                    value: ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1))
                },
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn enumerated() {
        let parse = |s| declaration().parse(s);

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
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn function() {
        let parse = |s| declaration().parse(s);

        assert_eq!(
            parse("func foo -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: None,
                    body: ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1))
                },
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("func foo: nil -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: Some(TypeExpression::Nil),
                    body: ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1))
                },
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("func foo() -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: None,
                    body: ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1))
                },
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("func foo(): nil -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body_type: Some(TypeExpression::Nil),
                    body: ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1))
                },
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn view() {
        let parse = |s| declaration().parse(s);

        assert_eq!(
            parse("view foo -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::View {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body: ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1))
                },
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("view foo() -> nil;").unwrap().0,
            DeclarationRaw(
                Declaration::View {
                    name: Storage(Visibility::Public, String::from("foo")),
                    parameters: vec![],
                    body: ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1))
                },
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn module() {
        let parse = |s| declaration().parse(s);

        assert_eq!(
            parse("module foo {}").unwrap().0,
            DeclarationRaw(
                Declaration::Module {
                    name: Storage(Visibility::Public, String::from("foo")),
                    value: ModuleRaw(Module::new(vec![], vec![]))
                },
                Range::str(1, 1)
            )
        );

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
                                    Range::str(1, 1)
                                )
                            },
                            Range::str(1, 1)
                        )]
                    ),)
                },
                Range::str(1, 1)
            )
        );
    }
}
