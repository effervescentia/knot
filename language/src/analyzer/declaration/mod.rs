mod fragment;
mod identify;
mod weak;
use super::{
    context::NodeContext,
    register::{Identify, Register},
    ScopeContext,
};
use crate::parser::{
    declaration::{self, DeclarationNode},
    node::Node,
    position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Register for DeclarationNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Node = DeclarationNode<T, NodeContext>;
    type Value<C> = declaration::NodeValue<T, C>;

    fn register(self, ctx: &mut ScopeContext) -> DeclarationNode<T, NodeContext> {
        let node = self.0;
        let value = node.0.identify(&mut ctx.child());
        let id = ctx.add_fragment(&value);

        DeclarationNode(Node(value, node.1, id))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::Register},
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
