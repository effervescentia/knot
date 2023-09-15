mod fragment;
mod identify;
mod weak;
use super::{
    context::NodeContext,
    register::{Identify, Register},
    RefKind, ScopeContext, Type, WeakType,
};
use crate::parser::{
    expression::ksx::{self, KSXNode},
    node::Node,
    position::Decrement,
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

    fn register(self, ctx: &mut ScopeContext) -> Self::Node {
        let node = self.0;
        let value = node.0.identify(ctx);
        let id = ctx.add_fragment(&value);

        KSXNode(Node(value, node.1, id))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, register::Register},
        parser::expression::{ksx::KSX, primitive::Primitive, Expression},
        test::fixture as f,
    };
    use std::collections::BTreeMap;

    #[test]
    fn text() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::kx(KSX::Text(String::from("foo"))).register(scope),
            f::n::kxc(KSX::Text(String::from("foo")), NodeContext::new(0, vec![0]))
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![(
                0,
                (vec![0], Fragment::KSX(KSX::Text(String::from("foo"))))
            )])
        );
    }

    #[test]
    fn inline() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::kx(KSX::Inline(f::n::x(Expression::Primitive(Primitive::Nil)))).register(scope),
            f::n::kxc(
                KSX::Inline(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (1, (vec![0], Fragment::KSX(KSX::Inline(0))))
            ])
        );
    }

    #[test]
    fn fragment() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::kx(KSX::Fragment(vec![f::n::kx(KSX::Inline(f::n::x(
                Expression::Primitive(Primitive::Nil)
            )))]))
            .register(scope),
            f::n::kxc(
                KSX::Fragment(vec![f::n::kxc(
                    KSX::Inline(f::n::xc(
                        Expression::Primitive(Primitive::Nil),
                        NodeContext::new(0, vec![0])
                    )),
                    NodeContext::new(1, vec![0]),
                )]),
                NodeContext::new(2, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (1, (vec![0], Fragment::KSX(KSX::Inline(0)))),
                (2, (vec![0], Fragment::KSX(KSX::Fragment(vec![1]))))
            ])
        );
    }

    #[test]
    fn closed_element() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::n::kx(KSX::ClosedElement(
                String::from("Foo"),
                vec![
                    (String::from("bar"), None),
                    (
                        String::from("fizz"),
                        Some(f::n::x(Expression::Primitive(Primitive::Nil))),
                    ),
                ],
            ))
            .register(scope),
            f::n::kxc(
                KSX::ClosedElement(
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
                ),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            BTreeMap::from_iter(vec![
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
                        Fragment::KSX(KSX::ClosedElement(
                            String::from("Foo"),
                            vec![(String::from("bar"), None), (String::from("fizz"), Some(0)),],
                        ))
                    )
                ),
            ])
        );
    }

    #[test]
    fn open_element() {
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
            BTreeMap::from_iter(vec![
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
