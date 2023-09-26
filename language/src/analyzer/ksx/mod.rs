mod fragment;
mod identify;
mod strong;
mod weak;

use super::{
    context::NodeContext,
    infer::weak::Weak,
    register::{Identify, Register},
    RefKind, ScopeContext, Type,
};
use crate::{
    ast::ksx::{self, KSXNode},
    common::{node::Node, position::Decrement},
};
use combine::Stream;
use std::fmt::Debug;

impl<T> Register for KSXNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Node = KSXNode<T, NodeContext>;
    type Value<C> = ksx::NodeValue<T, C>;

    fn register(&self, ctx: &ScopeContext) -> Self::Node {
        let value = self.node().value().identify(ctx);
        let id = ctx.add_fragment(&value);

        KSXNode(Node(value, self.node().range().clone(), id))
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
            expression::{Expression, Primitive},
            ksx::KSX,
        },
        test::fixture as f,
    };

    #[test]
    fn register() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::kx(KSX::OpenElement(
                String::from("Foo"),
                vec![
                    (String::from("bar"), None),
                    (
                        String::from("fizz"),
                        Some(f::n::x(Expression::Primitive(Primitive::Nil))),
                    ),
                ],
                vec![f::n::kx(KSX::Inline(f::n::x(Expression::Primitive(
                    Primitive::Nil
                ))))],
                String::from("Foo"),
            ))
            .register(scope),
            f::n::kxc(
                KSX::OpenElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(f::n::xc(
                                Expression::Primitive(Primitive::Nil),
                                NodeContext::new(0, vec![0])
                            )),
                        ),
                    ],
                    vec![f::n::kxc(
                        KSX::Inline(f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(1, vec![0])
                        )),
                        NodeContext::new(2, vec![0]),
                    )],
                    String::from("Foo"),
                ),
                NodeContext::new(3, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            FragmentMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    1,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (2, (vec![0], Fragment::KSX(KSX::Inline(1)))),
                (
                    3,
                    (
                        vec![0],
                        Fragment::KSX(KSX::OpenElement(
                            String::from("Foo"),
                            vec![(String::from("bar"), None), (String::from("fizz"), Some(0))],
                            vec![2],
                            String::from("Foo"),
                        ))
                    )
                ),
            ])
        );
    }
}
