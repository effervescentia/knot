use super::{Analyze, Context, Fragment};
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
                value_type: value_type.map(|x| x.register(ctx)),
                default_value: default_value.map(|x| x.register(ctx)),
            },
        )
        .collect::<Vec<_>>()
}

impl<T> Analyze<DeclarationNode<T, usize>, Declaration<usize, usize, usize>>
    for DeclarationNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Value<C> = Declaration<ExpressionNode<T, C>, ModuleNode<T, C>, TypeExpressionNode<T, C>>;

    fn register(self, ctx: &mut Context) -> DeclarationNode<T, usize> {
        let node = self.0;
        let value = Self::identify(node.0, ctx);
        let fragment = Fragment::Declaration(Self::to_ref(&value));
        let id = ctx.register(fragment);

        DeclarationNode(Node(value, node.1, id))
    }

    fn identify(value: Self::Value<()>, ctx: &mut Context) -> Self::Value<usize> {
        match value {
            Declaration::TypeAlias { name, value } => Declaration::TypeAlias {
                name,
                value: value.register(ctx),
            },

            Declaration::Enumerated { name, variants } => Declaration::Enumerated {
                name,
                variants: variants
                    .into_iter()
                    .map(|(name, xs)| {
                        (
                            name,
                            xs.into_iter().map(|x| x.register(ctx)).collect::<Vec<_>>(),
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
                value_type: value_type.map(|x| x.register(ctx)),
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
                body_type: body_type.map(|x| x.register(ctx)),
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
        }
    }

    fn to_ref<'a>(value: &'a Self::Value<usize>) -> Declaration<usize, usize, usize> {
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

        match value {
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

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{Analyze, Context, Fragment},
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
    use std::collections::HashMap;

    #[test]
    fn type_alias() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::dc(
                Declaration::TypeAlias {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    value: f::txc(TypeExpression::Nil, ()),
                },
                (),
            )
            .register(ctx),
            f::dc(
                Declaration::TypeAlias {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    value: f::txc(TypeExpression::Nil, 0),
                },
                1,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (0, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    1,
                    Fragment::Declaration(Declaration::TypeAlias {
                        name: Storage(Visibility::Public, String::from("Foo")),
                        value: 0
                    })
                )
            ])
        );
    }

    #[test]
    fn enumerated() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::dc(
                Declaration::Enumerated {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    variants: vec![(String::from("Bar"), vec![f::txc(TypeExpression::Nil, ())])],
                },
                (),
            )
            .register(ctx),
            f::dc(
                Declaration::Enumerated {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    variants: vec![(String::from("Bar"), vec![f::txc(TypeExpression::Nil, 0)])],
                },
                1,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (0, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    1,
                    Fragment::Declaration(Declaration::Enumerated {
                        name: Storage(Visibility::Public, String::from("Foo")),
                        variants: vec![(String::from("Bar"), vec![0])],
                    })
                )
            ])
        );
    }

    #[test]
    fn constant() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::dc(
                Declaration::Constant {
                    name: Storage(Visibility::Public, String::from("FOO")),
                    value_type: Some(f::txc(TypeExpression::Nil, ())),
                    value: f::xc(Expression::Primitive(Primitive::Nil), ()),
                },
                (),
            )
            .register(ctx),
            f::dc(
                Declaration::Constant {
                    name: Storage(Visibility::Public, String::from("FOO")),
                    value_type: Some(f::txc(TypeExpression::Nil, 0)),
                    value: f::xc(Expression::Primitive(Primitive::Nil), 1),
                },
                2,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (0, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    1,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    2,
                    Fragment::Declaration(Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("FOO")),
                        value_type: Some(0),
                        value: 1,
                    })
                )
            ])
        );
    }

    #[test]
    fn function() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::dc(
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
            .register(ctx),
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
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (0, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    1,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (2, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    3,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    4,
                    Fragment::Declaration(Declaration::Function {
                        name: Storage(Visibility::Public, String::from("Foo")),
                        parameters: vec![Parameter {
                            name: String::from("bar"),
                            value_type: Some(0),
                            default_value: Some(1),
                        }],
                        body_type: Some(2),
                        body: 3,
                    })
                )
            ])
        );
    }

    #[test]
    fn view() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::dc(
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
            .register(ctx),
            f::dc(
                Declaration::View {
                    name: Storage(Visibility::Public, String::from("Foo")),
                    parameters: vec![Parameter {
                        name: String::from("bar"),
                        value_type: Some(f::txc(TypeExpression::Nil, 0)),
                        default_value: Some(f::xc(Expression::Primitive(Primitive::Nil), 1)),
                    }],
                    body: f::xc(Expression::Primitive(Primitive::Nil), 2),
                },
                3,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (0, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    1,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    2,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    3,
                    Fragment::Declaration(Declaration::View {
                        name: Storage(Visibility::Public, String::from("Foo")),
                        parameters: vec![Parameter {
                            name: String::from("bar"),
                            value_type: Some(0),
                            default_value: Some(1),
                        }],
                        body: 2,
                    })
                )
            ])
        );
    }

    #[test]
    fn module() {
        let ctx = &mut Context::new();

        assert_eq!(
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
            .register(ctx),
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
                                    value_type: Some(f::txc(TypeExpression::Nil, 0)),
                                    value: f::xc(Expression::Primitive(Primitive::Nil), 1),
                                },
                                2,
                            )],
                        },
                        3,
                    ),
                },
                4,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (0, Fragment::TypeExpression(TypeExpression::Nil)),
                (
                    1,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    2,
                    Fragment::Declaration(Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("BUZZ")),
                        value_type: Some(0),
                        value: 1,
                    })
                ),
                (
                    3,
                    Fragment::Module(Module {
                        imports: vec![Import {
                            source: Source::Root,
                            path: vec![String::from("bar"), String::from("fizz")],
                            aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                        }],
                        declarations: vec![2],
                    })
                ),
                (
                    4,
                    Fragment::Declaration(Declaration::Module {
                        name: Storage(Visibility::Public, String::from("Foo")),
                        value: 3,
                    })
                )
            ])
        );
    }
}
