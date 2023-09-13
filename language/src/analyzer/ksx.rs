use super::{context::NodeContext, fragment::Fragment, Analyze, ScopeContext};
use crate::parser::{
    expression::{
        ksx::{KSXNode, KSX},
        ExpressionNode,
    },
    node::Node,
    position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

fn identify_attributes<T>(
    xs: Vec<(String, Option<ExpressionNode<T, ()>>)>,
    ctx: &mut ScopeContext,
) -> Vec<(String, Option<ExpressionNode<T, NodeContext>>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    xs.into_iter()
        .map(|(key, value)| (key, value.map(|x| x.register(ctx))))
        .collect::<Vec<_>>()
}

fn identify_children<T>(
    xs: Vec<KSXNode<T, ()>>,
    ctx: &mut ScopeContext,
) -> Vec<KSXNode<T, NodeContext>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    xs.into_iter().map(|x| x.register(ctx)).collect::<Vec<_>>()
}

impl<T> Analyze<KSXNode<T, NodeContext>, KSX<usize, usize>> for KSXNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Value<C> = KSX<ExpressionNode<T, C>, KSXNode<T, C>>;

    fn register(self, ctx: &mut ScopeContext) -> KSXNode<T, NodeContext> {
        let node = self.0;
        let value = Self::identify(node.0, ctx);
        let fragment = Fragment::KSX(Self::to_ref(&value));
        let id = ctx.add_fragment(fragment);

        KSXNode(Node(value, node.1, id))
    }

    fn identify(value: Self::Value<()>, ctx: &mut ScopeContext) -> Self::Value<NodeContext> {
        match value {
            KSX::Text(x) => KSX::Text(x),

            KSX::Inline(x) => KSX::Inline(x.register(ctx)),

            KSX::Fragment(children) => KSX::Fragment(identify_children(children, ctx)),

            KSX::ClosedElement(tag, attributes) => {
                KSX::ClosedElement(tag, identify_attributes(attributes, ctx))
            }

            KSX::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                start_tag,
                identify_attributes(attributes, ctx),
                identify_children(children, ctx),
                end_tag,
            ),
        }
    }

    fn to_ref<'a>(value: &'a Self::Value<NodeContext>) -> KSX<usize, usize> {
        let attributes_to_refs = |xs: &Vec<(String, Option<ExpressionNode<T, NodeContext>>)>| {
            xs.into_iter()
                .map(|(key, value)| (key.clone(), value.as_ref().map(|x| *x.node().id())))
                .collect::<Vec<_>>()
        };

        match value {
            KSX::Text(x) => KSX::Text(x.clone()),

            KSX::Inline(x) => KSX::Inline(*x.0.id()),

            KSX::Fragment(xs) => {
                KSX::Fragment(xs.into_iter().map(|x| *x.0.id()).collect::<Vec<_>>())
            }

            KSX::ClosedElement(tag, attributes) => {
                KSX::ClosedElement(tag.clone(), attributes_to_refs(attributes))
            }

            KSX::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                start_tag.clone(),
                attributes_to_refs(attributes),
                children.into_iter().map(|x| *x.0.id()).collect::<Vec<_>>(),
                end_tag.clone(),
            ),
        }
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, fragment::Fragment, Analyze},
        parser::expression::{ksx::KSX, primitive::Primitive, Expression},
        test::fixture as f,
    };
    use std::collections::HashMap;

    #[test]
    fn text() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::kxc(KSX::Text(String::from("foo")), ()).register(scope),
            f::kxc(KSX::Text(String::from("foo")), NodeContext::new(0, vec![0]))
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![(0, Fragment::KSX(KSX::Text(String::from("foo"))))])
        );
    }

    #[test]
    fn inline() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::kxc(
                KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), ())),
                (),
            )
            .register(scope),
            f::kxc(
                KSX::Inline(f::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
                NodeContext::new(1, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (1, Fragment::KSX(KSX::Inline(0)))
            ])
        );
    }

    #[test]
    fn fragment() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::kxc(
                KSX::Fragment(vec![f::kxc(
                    KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), ())),
                    (),
                )]),
                (),
            )
            .register(scope),
            f::kxc(
                KSX::Fragment(vec![f::kxc(
                    KSX::Inline(f::xc(
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
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (1, Fragment::KSX(KSX::Inline(0))),
                (2, Fragment::KSX(KSX::Fragment(vec![1])))
            ])
        );
    }

    #[test]
    fn closed_element() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::kxc(
                KSX::ClosedElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(f::xc(Expression::Primitive(Primitive::Nil), ())),
                        ),
                    ],
                ),
                (),
            )
            .register(scope),
            f::kxc(
                KSX::ClosedElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(f::xc(
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
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    1,
                    Fragment::KSX(KSX::ClosedElement(
                        String::from("Foo"),
                        vec![(String::from("bar"), None), (String::from("fizz"), Some(0)),],
                    ))
                ),
            ])
        );
    }

    #[test]
    fn open_element() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            f::kxc(
                KSX::OpenElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(f::xc(Expression::Primitive(Primitive::Nil), ())),
                        ),
                    ],
                    vec![f::kxc(
                        KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), ())),
                        (),
                    )],
                    String::from("Foo"),
                ),
                (),
            )
            .register(scope),
            f::kxc(
                KSX::OpenElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(f::xc(
                                Expression::Primitive(Primitive::Nil),
                                NodeContext::new(0, vec![0])
                            )),
                        ),
                    ],
                    vec![f::kxc(
                        KSX::Inline(f::xc(
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
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    1,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (2, Fragment::KSX(KSX::Inline(1))),
                (
                    3,
                    Fragment::KSX(KSX::OpenElement(
                        String::from("Foo"),
                        vec![(String::from("bar"), None), (String::from("fizz"), Some(0)),],
                        vec![2],
                        String::from("Foo"),
                    ))
                ),
            ])
        );
    }
}
