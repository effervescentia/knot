mod fragment;
mod identify;
mod strong;
mod weak;

use super::{
    context::NodeContext,
    register::{Identify, Register},
    ScopeContext,
};
use crate::{
    ast::{DeclarationNode, DeclarationNodeValue},
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Register for DeclarationNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Node = DeclarationNode<T, NodeContext>;
    type Value<C> = DeclarationNodeValue<T, C>;

    fn register(&self, ctx: &ScopeContext) -> DeclarationNode<T, NodeContext> {
        let value = self.node().value().identify(&mut ctx.child());
        let id = ctx.add_fragment(&value);

        DeclarationNode(Node(value, self.node().range().clone(), id))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{
            context::{FragmentMap, NodeContext},
            fragment::Fragment,
            register::Register,
        },
        ast::{
            Expression, Import, ImportSource, ImportTarget, Module, ModuleNode, Parameter,
            Primitive, TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn register_declaration() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            f::n::d(f::a::func_(
                "foo",
                vec![f::n::p(Parameter::new(
                    String::from("bar"),
                    Some(f::n::tx(TypeExpression::Nil)),
                    Some(f::n::x(Expression::Primitive(Primitive::Nil))),
                ),)],
                Some(f::n::tx(TypeExpression::Nil)),
                f::n::x(Expression::Primitive(Primitive::Nil)),
            ))
            .register(scope),
            f::n::dc(
                f::a::func_(
                    "foo",
                    vec![f::n::pc(
                        Parameter::new(
                            String::from("bar"),
                            Some(f::n::txc(
                                TypeExpression::Nil,
                                NodeContext::new(0, vec![0, 1])
                            )),
                            Some(f::n::xc(
                                Expression::Primitive(Primitive::Nil),
                                NodeContext::new(1, vec![0, 1])
                            )),
                        ),
                        NodeContext::new(2, vec![0, 1])
                    )],
                    Some(f::n::txc(
                        TypeExpression::Nil,
                        NodeContext::new(3, vec![0, 1])
                    )),
                    f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(4, vec![0, 1])
                    )
                ),
                NodeContext::new(5, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            FragmentMap::from_iter(vec![
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
                        Fragment::Parameter(Parameter::new(String::from("bar"), Some(0), Some(1)))
                    )
                ),
                (
                    3,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    4,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    5,
                    (
                        vec![0],
                        Fragment::Declaration(f::a::func_("foo", vec![2], Some(3), 4))
                    )
                )
            ])
        );
    }

    #[test]
    fn register_module() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            f::n::d(f::a::module(
                "foo",
                f::n::mr(Module::new(
                    vec![Import {
                        source: ImportSource::Root,
                        path: vec![String::from("bar"), String::from("fizz")],
                        aliases: Some(vec![(ImportTarget::Module, Some(String::from("Fizz")))]),
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
                f::a::module(
                    "foo",
                    ModuleNode(
                        Module::new(
                            vec![Import {
                                source: ImportSource::Root,
                                path: vec![String::from("bar"), String::from("fizz")],
                                aliases: Some(vec![(
                                    ImportTarget::Module,
                                    Some(String::from("Fizz"))
                                )]),
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
            FragmentMap::from_iter(vec![
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
                                source: ImportSource::Root,
                                path: vec![String::from("bar"), String::from("fizz")],
                                aliases: Some(vec![(
                                    ImportTarget::Module,
                                    Some(String::from("Fizz"))
                                )]),
                            }],
                            vec![2],
                        ))
                    )
                ),
                (4, (vec![0], Fragment::Declaration(f::a::module("foo", 3))))
            ])
        );
    }
}
