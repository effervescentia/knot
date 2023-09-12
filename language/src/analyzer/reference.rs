use super::Context;
use crate::parser::{
    declaration::{parameter::Parameter, Declaration, DeclarationNode},
    expression::{
        ksx::{KSXNode, KSX},
        statement::Statement,
        Expression, ExpressionNode,
    },
    module::{Module, ModuleNode},
    position::Decrement,
    types::type_expression::{TypeExpression, TypeExpressionNode},
};
use combine::Stream;
use std::fmt::Debug;

pub trait ToRef<'a, T>: Sized {
    fn to_ref(&'a self) -> T;
}

trait Register<T>: Sized {
    fn register(self, ctx: &mut Context) -> T;
}

impl<'a, T> ToRef<'a, Expression<usize, usize>>
    for Expression<ExpressionNode<T, usize>, KSXNode<T, usize>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_ref(&'a self) -> Expression<usize, usize> {
        match self {
            Expression::Primitive(x) => Expression::Primitive(x.clone()),

            Expression::Identifier(x) => Expression::Identifier(x.clone()),

            Expression::Group(x) => Expression::Group(Box::new(x.node().id())),

            Expression::Closure(xs) => Expression::Closure(
                xs.into_iter()
                    .map(|x| match x {
                        Statement::Effect(x) => Statement::Effect(x.0.id()),
                        Statement::Variable(name, x) => Statement::Variable(name.clone(), x.0.id()),
                    })
                    .collect::<Vec<_>>(),
            ),

            Expression::UnaryOperation(op, x) => {
                Expression::UnaryOperation(op.clone(), Box::new(x.0.id()))
            }

            Expression::BinaryOperation(op, lhs, rhs) => {
                Expression::BinaryOperation(op.clone(), Box::new(lhs.0.id()), Box::new(rhs.0.id()))
            }

            Expression::DotAccess(lhs, rhs) => {
                Expression::DotAccess(Box::new(lhs.0.id()), rhs.clone())
            }

            Expression::FunctionCall(x, args) => Expression::FunctionCall(
                Box::new(x.0.id()),
                args.into_iter().map(|x| x.0.id()).collect::<Vec<_>>(),
            ),

            Expression::Style(xs) => Expression::Style(
                xs.into_iter()
                    .map(|(key, value)| (key.clone(), value.0.id()))
                    .collect::<Vec<_>>(),
            ),

            Expression::KSX(x) => Expression::KSX(Box::new(x.0.id())),
        }
    }
}

impl<'a, T> ToRef<'a, KSX<usize, usize>> for KSX<ExpressionNode<T, usize>, KSXNode<T, usize>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_ref(&'a self) -> KSX<usize, usize> {
        let attributes_to_refs = |xs: &Vec<(String, Option<ExpressionNode<T, usize>>)>| {
            xs.into_iter()
                .map(|(key, value)| (key.clone(), value.as_ref().map(|x| x.node().id())))
                .collect::<Vec<_>>()
        };

        match self {
            KSX::Text(x) => KSX::Text(x.clone()),

            KSX::Inline(x) => KSX::Inline(x.0.id()),

            KSX::Fragment(xs) => {
                KSX::Fragment(xs.into_iter().map(|x| x.0.id()).collect::<Vec<_>>())
            }

            KSX::ClosedElement(tag, attributes) => {
                KSX::ClosedElement(tag.clone(), attributes_to_refs(attributes))
            }

            KSX::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                start_tag.clone(),
                attributes_to_refs(attributes),
                children.into_iter().map(|x| x.0.id()).collect::<Vec<_>>(),
                end_tag.clone(),
            ),
        }
    }
}

impl<'a, T> ToRef<'a, TypeExpression<usize>> for TypeExpression<TypeExpressionNode<T, usize>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_ref(&'a self) -> TypeExpression<usize> {
        match self {
            TypeExpression::Nil => TypeExpression::Nil,
            TypeExpression::Boolean => TypeExpression::Boolean,
            TypeExpression::Integer => TypeExpression::Integer,
            TypeExpression::Float => TypeExpression::Float,
            TypeExpression::String => TypeExpression::String,
            TypeExpression::Style => TypeExpression::Style,
            TypeExpression::Element => TypeExpression::Element,

            TypeExpression::Identifier(x) => TypeExpression::Identifier(x.clone()),

            TypeExpression::Group(x) => TypeExpression::Group(Box::new((*x).0.id())),

            TypeExpression::DotAccess(lhs, rhs) => {
                TypeExpression::DotAccess(Box::new((*lhs).0.id()), rhs.clone())
            }

            TypeExpression::Function(params, body) => TypeExpression::Function(
                params.into_iter().map(|x| x.0.id()).collect::<Vec<_>>(),
                Box::new(body.0.id()),
            ),
        }
    }
}

impl<'a, T> ToRef<'a, Declaration<usize, usize, usize>>
    for Declaration<ExpressionNode<T, usize>, ModuleNode<T, usize>, TypeExpressionNode<T, usize>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_ref(&'a self) -> Declaration<usize, usize, usize> {
        let parameters_to_refs =
            |xs: &Vec<Parameter<ExpressionNode<T, usize>, TypeExpressionNode<T, usize>>>| {
                xs.into_iter()
                    .map(
                        |Parameter {
                             name,
                             value_type,
                             default_value,
                         }| Parameter {
                            name: name.clone(),
                            value_type: value_type.as_ref().map(|x| x.node().id()),
                            default_value: default_value.as_ref().map(|x| x.node().id()),
                        },
                    )
                    .collect::<Vec<_>>()
            };

        match self {
            Declaration::TypeAlias { name, value } => Declaration::TypeAlias {
                name: name.clone(),
                value: value.0.id(),
            },

            Declaration::Enumerated { name, variants } => Declaration::Enumerated {
                name: name.clone(),
                variants: variants
                    .into_iter()
                    .map(|(name, params)| {
                        (
                            name.clone(),
                            params
                                .into_iter()
                                .map(|x| x.node().id())
                                .collect::<Vec<_>>(),
                        )
                    })
                    .collect::<Vec<_>>(),
            },

            Declaration::Constant {
                name,
                value_type,
                value,
            } => Declaration::Constant {
                name: name.clone(),
                value_type: value_type.as_ref().map(|x| x.node().id()),
                value: value.0.id(),
            },

            Declaration::Function {
                name,
                parameters,
                body_type,
                body,
            } => Declaration::Function {
                name: name.clone(),
                parameters: parameters_to_refs(parameters),
                body_type: body_type.as_ref().map(|x| x.node().id()),
                body: body.node().id(),
            },

            Declaration::View {
                name,
                parameters,
                body,
            } => Declaration::View {
                name: name.clone(),
                parameters: parameters_to_refs(parameters),
                body: body.node().id(),
            },

            Declaration::Module { name, value } => Declaration::Module {
                name: name.clone(),
                value: value.id(),
            },
        }
    }
}

impl<T> ModuleNode<T, usize>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn id(&self) -> usize {
        self.1
    }
}

impl<'a, T> ToRef<'a, Module<usize>> for Module<DeclarationNode<T, usize>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_ref(&'a self) -> Module<usize> {
        Module {
            imports: self.imports.iter().map(|x| x.clone()).collect::<Vec<_>>(),
            declarations: self
                .declarations
                .iter()
                .map(|x| x.0.id())
                .collect::<Vec<_>>(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::ToRef;
    use crate::{
        parser::{
            declaration::{
                parameter::Parameter,
                storage::{Storage, Visibility},
                Declaration,
            },
            expression::{
                binary_operation::BinaryOperator, ksx::KSX, primitive::Primitive,
                statement::Statement, Expression, ExpressionNode, UnaryOperator,
            },
            module::{
                import::{Import, Source},
                Module, ModuleNode,
            },
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn expression_primitive() {
        let input = f::xc(Expression::Primitive(Primitive::Nil), 0);

        assert_eq!(
            input.node().value().to_ref(),
            Expression::Primitive(Primitive::Nil)
        )
    }

    #[test]
    fn expression_identifier() {
        let input = f::xc(Expression::Identifier(String::from("foo")), 0);

        assert_eq!(
            input.node().value().to_ref(),
            Expression::Identifier(String::from("foo"))
        )
    }

    #[test]
    fn expression_group() {
        let input = f::xc(
            Expression::Group(Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0))),
            1,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Expression::Group(Box::new(0))
        )
    }

    #[test]
    fn expression_closure() {
        let input = f::xc(
            Expression::Closure(vec![
                Statement::Variable(
                    String::from("foo"),
                    f::xc(Expression::Primitive(Primitive::Nil), 0),
                ),
                Statement::Effect(f::xc(Expression::Primitive(Primitive::Nil), 1)),
            ]),
            2,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Expression::Closure(vec![
                Statement::Variable(String::from("foo"), 0),
                Statement::Effect(1),
            ])
        )
    }

    #[test]
    fn expression_unary_operation() {
        let input = f::xc(
            Expression::UnaryOperation(
                UnaryOperator::Not,
                Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
            ),
            1,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Expression::UnaryOperation(UnaryOperator::Not, Box::new(0))
        )
    }

    #[test]
    fn expression_binary_operation() {
        let input: ExpressionNode<
            combine::easy::Stream<
                combine::stream::position::Stream<&str, combine::stream::position::SourcePosition>,
            >,
            usize,
        > = f::xc(
            Expression::BinaryOperation(
                BinaryOperator::Equal,
                Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                Box::new(f::xc(Expression::Primitive(Primitive::Nil), 1)),
            ),
            2,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Expression::BinaryOperation(BinaryOperator::Equal, Box::new(0), Box::new(1))
        )
    }

    #[test]
    fn expression_dot_access() {
        let input = f::xc(
            Expression::DotAccess(
                Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                String::from("foo"),
            ),
            1,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Expression::DotAccess(Box::new(0), String::from("foo"))
        )
    }

    #[test]
    fn expression_function_call() {
        let input = f::xc(
            Expression::FunctionCall(
                Box::new(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                vec![
                    f::xc(Expression::Primitive(Primitive::Nil), 1),
                    f::xc(Expression::Primitive(Primitive::Nil), 2),
                ],
            ),
            3,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Expression::FunctionCall(Box::new(0), vec![1, 2])
        )
    }

    #[test]
    fn expression_style() {
        let input = f::xc(
            Expression::Style(vec![
                (
                    String::from("foo"),
                    f::xc(Expression::Primitive(Primitive::Nil), 0),
                ),
                (
                    String::from("bar"),
                    f::xc(Expression::Primitive(Primitive::Nil), 1),
                ),
            ]),
            2,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Expression::Style(vec![(String::from("foo"), 0), (String::from("bar"), 1)])
        )
    }

    #[test]
    fn expression_ksx() {
        let input = f::xc(
            Expression::KSX(Box::new(f::kxc(KSX::Fragment(vec![]), 0))),
            1,
        );

        assert_eq!(input.node().value().to_ref(), Expression::KSX(Box::new(0)))
    }

    #[test]
    fn ksx_text() {
        let input = f::kxc(KSX::Text(String::from("foo")), 0);

        assert_eq!(
            input.node().value().to_ref(),
            KSX::Text(String::from("foo"))
        )
    }

    #[test]
    fn ksx_inline() {
        let input = f::kxc(
            KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), 0)),
            1,
        );

        assert_eq!(input.node().value().to_ref(), KSX::Inline(0))
    }

    #[test]
    fn ksx_fragment() {
        let input = f::kxc(
            KSX::Fragment(vec![f::kxc(KSX::Text(String::from("foo")), 0)]),
            1,
        );

        assert_eq!(input.node().value().to_ref(), KSX::Fragment(vec![0]))
    }

    #[test]
    fn ksx_closed_element() {
        let input = f::kxc(
            KSX::ClosedElement(
                String::from("Foo"),
                vec![
                    (String::from("bar"), None),
                    (
                        String::from("fizz"),
                        Some(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                    ),
                ],
            ),
            2,
        );

        assert_eq!(
            input.node().value().to_ref(),
            KSX::ClosedElement(
                String::from("Foo"),
                vec![(String::from("bar"), None), (String::from("fizz"), Some(0))],
            )
        )
    }

    #[test]
    fn ksx_open_element() {
        let input = f::kxc(
            KSX::OpenElement(
                String::from("Foo"),
                vec![
                    (String::from("bar"), None),
                    (
                        String::from("fizz"),
                        Some(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                    ),
                ],
                vec![f::kxc(KSX::Text(String::from("buzz")), 1)],
                String::from("Foo"),
            ),
            2,
        );

        assert_eq!(
            input.node().value().to_ref(),
            KSX::OpenElement(
                String::from("Foo"),
                vec![(String::from("bar"), None), (String::from("fizz"), Some(0))],
                vec![1],
                String::from("Foo")
            )
        )
    }

    #[test]
    fn declaration_type_alias() {
        let input = f::dc(
            Declaration::TypeAlias {
                name: Storage(Visibility::Public, String::from("FOO")),
                value: f::txc(TypeExpression::Nil, 0),
            },
            2,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Declaration::TypeAlias {
                name: Storage(Visibility::Public, String::from("FOO")),
                value: 0
            }
        )
    }

    #[test]
    fn declaration_enumerated() {
        let input = f::dc(
            Declaration::Enumerated {
                name: Storage(Visibility::Public, String::from("Foo")),
                variants: vec![(String::from("Bar"), vec![f::txc(TypeExpression::Nil, 0)])],
            },
            1,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Declaration::Enumerated {
                name: Storage(Visibility::Public, String::from("Foo")),
                variants: vec![(String::from("Bar"), vec![0])]
            }
        )
    }

    #[test]
    fn declaration_constant() {
        let input = f::dc(
            Declaration::Constant {
                name: Storage(Visibility::Public, String::from("FOO")),
                value_type: Some(f::txc(TypeExpression::Nil, 0)),
                value: f::xc(Expression::Primitive(Primitive::Nil), 1),
            },
            2,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Declaration::Constant {
                name: Storage(Visibility::Public, String::from("FOO")),
                value_type: Some(0),
                value: 1
            }
        )
    }

    #[test]
    fn declaration_function() {
        let input = f::dc(
            Declaration::Function {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(f::txc(TypeExpression::Nil, 0)),
                    default_value: Some(f::xc(Expression::Primitive(Primitive::Nil), 1)),
                }],
                body_type: Some(f::txc(TypeExpression::Nil, 2)),
                body: f::xc(Expression::Primitive(Primitive::Nil), 3),
            },
            4,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Declaration::Function {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(0),
                    default_value: Some(1),
                }],
                body_type: Some(2),
                body: 3
            }
        )
    }

    #[test]
    fn declaration_view() {
        let input = f::dc(
            Declaration::View {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(f::txc(TypeExpression::Nil, 0)),
                    default_value: Some(f::xc(Expression::Primitive(Primitive::Nil), 1)),
                }],
                body: f::xc(Expression::Primitive(Primitive::Nil), 2),
            },
            3,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Declaration::View {
                name: Storage(Visibility::Public, String::from("foo")),
                parameters: vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(0),
                    default_value: Some(1),
                }],
                body: 2
            }
        )
    }

    #[test]
    fn declaration_module() {
        let input = f::dc(
            Declaration::Module {
                name: Storage(Visibility::Public, String::from("foo")),
                value: ModuleNode(
                    Module {
                        imports: vec![],
                        declarations: vec![],
                    },
                    0,
                ),
            },
            1,
        );

        assert_eq!(
            input.node().value().to_ref(),
            Declaration::Module {
                name: Storage(Visibility::Public, String::from("foo")),
                value: 0,
            }
        )
    }

    #[test]
    fn module() {
        let input = ModuleNode(
            Module {
                imports: vec![Import::new(Source::Root, vec![String::from("foo")], None)],
                declarations: vec![f::dc(
                    Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("FOO")),
                        value_type: Some(f::txc(TypeExpression::Nil, 0)),
                        value: f::xc(Expression::Primitive(Primitive::Nil), 1),
                    },
                    2,
                )],
            },
            3,
        );

        assert_eq!(
            input.0.to_ref(),
            Module {
                imports: vec![Import::new(Source::Root, vec![String::from("foo")], None)],
                declarations: vec![2],
            }
        )
    }
}
