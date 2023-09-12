use super::{reference::ToRef, type_expression, Context, Fragment, Register};
use crate::parser::{
    declaration::{parameter::Parameter, Declaration, DeclarationNode},
    expression::ExpressionNode,
    module::ModuleNode,
    node::Node,
    position::Decrement,
    types::type_expression::TypeExpressionNode,
};
use combine::Stream;
use std::fmt::Debug;

fn identify_parameters<T>(
    xs: Vec<Parameter<ExpressionNode<T, ()>, TypeExpressionNode<T, ()>>>,
    ctx: &mut Context,
) -> Vec<Parameter<ExpressionNode<T, usize>, TypeExpressionNode<T, usize>>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    xs.into_iter()
        .map(
            |Parameter {
                 name,
                 value_type,
                 default_value,
             }| Parameter {
                name,
                value_type: value_type.map(|x| type_expression::analyze_type_expression(x, ctx)),
                default_value: default_value.map(|x| x.register(ctx)),
            },
        )
        .collect::<Vec<_>>()
}

fn identify_declaration<T>(
    x: DeclarationNode<T, ()>,
    ctx: &mut Context,
) -> Node<
    Declaration<ExpressionNode<T, usize>, ModuleNode<T, usize>, TypeExpressionNode<T, usize>>,
    T,
    (),
>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    x.0.map(|x| match x {
        Declaration::TypeAlias { name, value } => Declaration::TypeAlias {
            name,
            value: type_expression::analyze_type_expression(value, ctx),
        },

        Declaration::Enumerated { name, variants } => Declaration::Enumerated {
            name,
            variants: variants
                .into_iter()
                .map(|(name, xs)| {
                    (
                        name,
                        xs.into_iter()
                            .map(|x| type_expression::analyze_type_expression(x, ctx))
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
            name,
            value_type: value_type.map(|x| type_expression::analyze_type_expression(x, ctx)),
            value: value.register(ctx),
        },

        Declaration::Function {
            name,
            parameters,
            body_type,
            body,
        } => Declaration::Function {
            name,
            parameters: identify_parameters(parameters, ctx),
            body_type: body_type.map(|x| type_expression::analyze_type_expression(x, ctx)),
            body: body.register(ctx),
        },

        Declaration::View {
            name,
            parameters,
            body,
        } => Declaration::View {
            name,
            parameters: identify_parameters(parameters, ctx),
            body: body.register(ctx),
        },

        Declaration::Module { name, value } => Declaration::Module {
            name,
            value: value.register(ctx),
        },
    })
}

impl<T> Register<DeclarationNode<T, usize>> for DeclarationNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn register(self, ctx: &mut Context) -> DeclarationNode<T, usize> {
        let node = identify_declaration(self, ctx);
        let fragment = Fragment::Declaration(node.value().to_ref());
        let id = ctx.register(fragment);

        DeclarationNode(node.with_context(id))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{Context, Register},
        parser::{
            declaration::{
                parameter::Parameter,
                storage::{Storage, Visibility},
                Declaration,
            },
            expression::{primitive::Primitive, Expression},
            module::{
                import::{Import, Source, Target},
                Module, ModuleNode,
            },
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn type_alias() {
        let ctx = &mut Context::new();

        let result = f::dc(
            Declaration::TypeAlias {
                name: Storage(Visibility::Public, String::from("Foo")),
                value: f::txc(TypeExpression::Nil, ()),
            },
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::dc(
                Declaration::TypeAlias {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    value: f::txc(TypeExpression::Nil, 0),
                },
                1,
            )
        )
    }

    #[test]
    fn enumerated() {
        let ctx = &mut Context::new();

        let result = f::dc(
            Declaration::Enumerated {
                name: Storage(Visibility::Public, String::from("Foo")),
                variants: vec![(String::from("Bar"), vec![f::txc(TypeExpression::Nil, ())])],
            },
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::dc(
                Declaration::Enumerated {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    variants: vec![(String::from("Bar"), vec![f::txc(TypeExpression::Nil, 0)],)],
                },
                1,
            )
        )
    }

    #[test]
    fn constant() {
        let ctx = &mut Context::new();

        let result = f::dc(
            Declaration::Constant {
                name: Storage(Visibility::Public, String::from("FOO")),
                value_type: Some(f::txc(TypeExpression::Nil, ())),
                value: f::xc(Expression::Primitive(Primitive::Nil), ()),
            },
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::dc(
                Declaration::Constant {
                    name: Storage(Visibility::Public, String::from("FOO")),
                    value_type: Some(f::txc(TypeExpression::Nil, 0)),
                    value: f::xc(Expression::Primitive(Primitive::Nil), 1),
                },
                2,
            )
        )
    }

    #[test]
    fn function() {
        let ctx = &mut Context::new();

        let result = f::dc(
            Declaration::Function {
                name: Storage(Visibility::Public, String::from("Foo")),
                parameters: vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(f::txc(TypeExpression::Nil, ())),
                    default_value: Some(f::xc(Expression::Primitive(Primitive::Nil), ())),
                }],
                body_type: Some(f::txc(TypeExpression::Nil, ())),
                body: f::xc(Expression::Primitive(Primitive::Nil), ()),
            },
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::dc(
                Declaration::Function {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    parameters: vec![Parameter {
                        name: String::from("bar"),
                        value_type: Some(f::txc(TypeExpression::Nil, 0)),
                        default_value: Some(f::xc(Expression::Primitive(Primitive::Nil), 1)),
                    }],
                    body_type: Some(f::txc(TypeExpression::Nil, 2)),
                    body: f::xc(Expression::Primitive(Primitive::Nil), 3),
                },
                4,
            )
        )
    }

    #[test]
    fn view() {
        let ctx = &mut Context::new();

        let result = f::dc(
            Declaration::View {
                name: Storage(Visibility::Public, String::from("Foo")),
                parameters: vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(f::txc(TypeExpression::Nil, ())),
                    default_value: Some(f::xc(Expression::Primitive(Primitive::Nil), ())),
                }],
                body: f::xc(Expression::Primitive(Primitive::Nil), ()),
            },
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::dc(
                Declaration::View {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    parameters: vec![Parameter {
                        name: String::from("bar"),
                        value_type: Some(f::txc(TypeExpression::Nil, 0)),
                        default_value: Some(f::xc(Expression::Primitive(Primitive::Nil), 1,)),
                    }],
                    body: f::xc(Expression::Primitive(Primitive::Nil), 2),
                },
                3,
            )
        )
    }

    #[test]
    fn module() {
        let ctx = &mut Context::new();

        let result = f::dc(
            Declaration::Module {
                name: Storage(Visibility::Public, String::from("Foo")),
                value: ModuleNode(
                    Module {
                        imports: vec![Import {
                            source: Source::Root,
                            path: vec![String::from("bar"), String::from("fizz")],
                            aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                        }],
                        declarations: vec![f::dc(
                            Declaration::Constant {
                                name: Storage(Visibility::Public, String::from("BUZZ")),
                                value_type: Some(f::txc(TypeExpression::Nil, ())),
                                value: f::xc(Expression::Primitive(Primitive::Nil), ()),
                            },
                            (),
                        )],
                    },
                    (),
                ),
            },
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::dc(
                Declaration::Module {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    value: ModuleNode(
                        Module {
                            imports: vec![Import {
                                source: Source::Root,
                                path: vec![String::from("bar"), String::from("fizz")],
                                aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                            }],
                            declarations: vec![f::dc(
                                Declaration::Constant {
                                    name: Storage(Visibility::Public, String::from("BUZZ")),
                                    value_type: Some(f::txc(TypeExpression::Nil, 0,)),
                                    value: f::xc(Expression::Primitive(Primitive::Nil), 1,),
                                },
                                2,
                            )],
                        },
                        3,
                    ),
                },
                4,
            )
        )
    }
}
