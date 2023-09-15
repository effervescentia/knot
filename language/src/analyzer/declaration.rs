use super::{context::NodeContext, fragment::Fragment, Analyze, ScopeContext};
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
    ctx: &mut ScopeContext,
) -> Vec<Parameter<ExpressionNode<T, NodeContext>, TypeExpressionNode<T, NodeContext>>>
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

impl<T> Analyze for DeclarationNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Ref = Declaration<usize, usize, usize>;
    type Node = DeclarationNode<T, NodeContext>;
    type Value<C> = Declaration<ExpressionNode<T, C>, ModuleNode<T, C>, TypeExpressionNode<T, C>>;

    fn register(self, ctx: &mut ScopeContext) -> DeclarationNode<T, NodeContext> {
        let node = self.0;
        let value = Self::identify(node.0, &mut ctx.child());
        let fragment = Fragment::Declaration(Self::to_ref(&value));
        let id = ctx.add_fragment(fragment);

        DeclarationNode(Node(value, node.1, id))
    }

    fn identify(value: Self::Value<()>, ctx: &mut ScopeContext) -> Self::Value<NodeContext> {
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

    fn to_ref<'a>(value: &'a Self::Value<NodeContext>) -> Declaration<usize, usize, usize> {
        let parameters_to_refs = |xs: &Vec<
            Parameter<ExpressionNode<T, NodeContext>, TypeExpressionNode<T, NodeContext>>,
        >| {
            xs.into_iter()
                .map(
                    |Parameter {
                         name,
                         value_type,
                         default_value,
                     }| Parameter {
                        name: name.clone(),
                        value_type: value_type.as_ref().map(|x| *x.node().id()),
                        default_value: default_value.as_ref().map(|x| *x.node().id()),
                    },
                )
                .collect::<Vec<_>>()
        };

        match value {
            Declaration::TypeAlias { name, value } => Declaration::TypeAlias {
                name: name.clone(),
                value: *value.0.id(),
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
                                .map(|x| *x.node().id())
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
                value_type: value_type.as_ref().map(|x| *x.node().id()),
                value: *value.0.id(),
            },

            Declaration::Function {
                name,
                parameters,
                body_type,
                body,
            } => Declaration::Function {
                name: name.clone(),
                parameters: parameters_to_refs(parameters),
                body_type: body_type.as_ref().map(|x| *x.node().id()),
                body: *body.node().id(),
            },

            Declaration::View {
                name,
                parameters,
                body,
            } => Declaration::View {
                name: name.clone(),
                parameters: parameters_to_refs(parameters),
                body: *body.node().id(),
            },

            Declaration::Module { name, value } => Declaration::Module {
                name: name.clone(),
                value: *value.id(),
            },
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, Analyze},
        parser::{
            declaration::parameter::Parameter,
            expression::{primitive::Primitive, Expression},
            module::{
                import::{Import, Source, Target},
                Module, ModuleNode,
            },
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };
    use std::collections::BTreeMap;

    #[test]
    fn type_alias() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::d(f::a::type_("Foo", f::n::tx(TypeExpression::Nil))).register(scope),
            f::n::dc(
                f::a::type_(
                    "Foo",
                    f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0, 1]))
                ),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (1, (vec![0], Fragment::Declaration(f::a::type_("Foo", 0))))
            ])
        );
    }

    #[test]
    fn enumerated() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::d(f::a::enum_(
                "Foo",
                vec![(String::from("Bar"), vec![f::n::tx(TypeExpression::Nil)])]
            ))
            .register(scope),
            f::n::dc(
                f::a::enum_(
                    "Foo",
                    vec![(
                        String::from("Bar"),
                        vec![f::n::txc(
                            TypeExpression::Nil,
                            NodeContext::new(0, vec![0, 1])
                        )]
                    )]
                ),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Declaration(f::a::enum_(
                            "Foo",
                            vec![(String::from("Bar"), vec![0])]
                        ))
                    )
                )
            ])
        );
    }

    #[test]
    fn constant() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::d(f::a::const_(
                "FOO",
                Some(f::n::tx(TypeExpression::Nil)),
                f::n::x(Expression::Primitive(Primitive::Nil))
            ))
            .register(scope),
            f::n::dc(
                f::a::const_(
                    "FOO",
                    Some(f::n::txc(
                        TypeExpression::Nil,
                        NodeContext::new(0, vec![0, 1])
                    )),
                    f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(1, vec![0, 1])
                    )
                ),
                NodeContext::new(2, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    1,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    2,
                    (
                        vec![0],
                        Fragment::Declaration(f::a::const_("FOO", Some(0), 1))
                    )
                )
            ])
        );
    }

    #[test]
    fn function() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::d(f::a::func_(
                "foo",
                vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(f::n::tx(TypeExpression::Nil)),
                    default_value: Some(f::n::x(Expression::Primitive(Primitive::Nil))),
                }],
                Some(f::n::tx(TypeExpression::Nil)),
                f::n::x(Expression::Primitive(Primitive::Nil)),
            ))
            .register(scope),
            f::n::dc(
                f::a::func_(
                    "foo",
                    vec![Parameter {
                        name: String::from("bar"),
                        value_type: Some(f::n::txc(
                            TypeExpression::Nil,
                            NodeContext::new(0, vec![0, 1])
                        )),
                        default_value: Some(f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(1, vec![0, 1])
                        )),
                    }],
                    Some(f::n::txc(
                        TypeExpression::Nil,
                        NodeContext::new(2, vec![0, 1])
                    )),
                    f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(3, vec![0, 1])
                    )
                ),
                NodeContext::new(4, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    1,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    2,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    3,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    4,
                    (
                        vec![0],
                        Fragment::Declaration(f::a::func_(
                            "foo",
                            vec![Parameter {
                                name: String::from("bar"),
                                value_type: Some(0),
                                default_value: Some(1),
                            }],
                            Some(2),
                            3,
                        ))
                    )
                )
            ])
        );
    }

    #[test]
    fn view() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::d(f::a::view(
                "Foo",
                vec![Parameter {
                    name: String::from("bar"),
                    value_type: Some(f::n::tx(TypeExpression::Nil)),
                    default_value: Some(f::n::x(Expression::Primitive(Primitive::Nil))),
                }],
                f::n::x(Expression::Primitive(Primitive::Nil)),
            ))
            .register(scope),
            f::n::dc(
                f::a::view(
                    "Foo",
                    vec![Parameter {
                        name: String::from("bar"),
                        value_type: Some(f::n::txc(
                            TypeExpression::Nil,
                            NodeContext::new(0, vec![0, 1])
                        )),
                        default_value: Some(f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(1, vec![0, 1])
                        )),
                    }],
                    f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(2, vec![0, 1])
                    ),
                ),
                NodeContext::new(3, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    1,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    2,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    3,
                    (
                        vec![0],
                        Fragment::Declaration(f::a::view(
                            "Foo",
                            vec![Parameter {
                                name: String::from("bar"),
                                value_type: Some(0),
                                default_value: Some(1),
                            }],
                            2,
                        ))
                    )
                )
            ])
        );
    }

    #[test]
    fn module() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::d(f::a::mod_(
                "foo",
                f::n::mr(Module::new(
                    vec![Import {
                        source: Source::Root,
                        path: vec![String::from("bar"), String::from("fizz")],
                        aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                    }],
                    vec![f::n::d(f::a::const_(
                        "BUZZ",
                        Some(f::n::tx(TypeExpression::Nil)),
                        f::n::x(Expression::Primitive(Primitive::Nil)),
                    ))],
                ))
            ))
            .register(scope),
            f::n::dc(
                f::a::mod_(
                    "foo",
                    ModuleNode(
                        Module::new(
                            vec![Import {
                                source: Source::Root,
                                path: vec![String::from("bar"), String::from("fizz")],
                                aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                            }],
                            vec![f::n::dc(
                                f::a::const_(
                                    "BUZZ",
                                    Some(f::n::txc(
                                        TypeExpression::Nil,
                                        NodeContext::new(0, vec![0, 1, 2])
                                    )),
                                    f::n::xc(
                                        Expression::Primitive(Primitive::Nil),
                                        NodeContext::new(1, vec![0, 1, 2])
                                    )
                                ),
                                NodeContext::new(2, vec![0, 1]),
                            )],
                        ),
                        NodeContext::new(3, vec![0, 1]),
                    )
                ),
                NodeContext::new(4, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (vec![0, 1, 2], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    1,
                    (
                        vec![0, 1, 2],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    2,
                    (
                        vec![0, 1],
                        Fragment::Declaration(f::a::const_("BUZZ", Some(0), 1))
                    )
                ),
                (
                    3,
                    (
                        vec![0, 1],
                        Fragment::Module(Module::new(
                            vec![Import {
                                source: Source::Root,
                                path: vec![String::from("bar"), String::from("fizz")],
                                aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                            }],
                            vec![2],
                        ))
                    )
                ),
                (4, (vec![0], Fragment::Declaration(f::a::mod_("foo", 3))))
            ])
        );
    }
}
