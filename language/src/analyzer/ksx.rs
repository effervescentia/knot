use super::{Analyze, Context};
use crate::{
    analyzer::Fragment,
    parser::{
        expression::{
            ksx::{KSXNode, KSX},
            ExpressionNode,
        },
        node::Node,
        position::Decrement,
    },
};
use combine::Stream;
use std::fmt::Debug;

fn identify_attributes<T>(
    xs: Vec<(String, Option<ExpressionNode<T, ()>>)>,
    ctx: &mut Context,
) -> Vec<(String, Option<ExpressionNode<T, usize>>)>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    xs.into_iter()
        .map(|(key, value)| (key, value.map(|x| x.register(ctx))))
        .collect::<Vec<_>>()
}

fn identify_children<T>(xs: Vec<KSXNode<T, ()>>, ctx: &mut Context) -> Vec<KSXNode<T, usize>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    xs.into_iter().map(|x| x.register(ctx)).collect::<Vec<_>>()
}

impl<T> Analyze<KSXNode<T, usize>, KSX<usize, usize>> for KSXNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Value<C> = KSX<ExpressionNode<T, C>, KSXNode<T, C>>;

    fn register(self, ctx: &mut Context) -> KSXNode<T, usize> {
        let node = self.0;
        let value = Self::identify(node.0, ctx);
        let fragment = Fragment::KSX(Self::to_ref(&value));
        let id = ctx.register(fragment);

        KSXNode(Node(value, node.1, id))
    }

    fn identify(value: Self::Value<()>, ctx: &mut Context) -> Self::Value<usize> {
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

    fn to_ref<'a>(value: &'a Self::Value<usize>) -> KSX<usize, usize> {
        let attributes_to_refs = |xs: &Vec<(String, Option<ExpressionNode<T, usize>>)>| {
            xs.into_iter()
                .map(|(key, value)| (key.clone(), value.as_ref().map(|x| x.node().id())))
                .collect::<Vec<_>>()
        };

        match value {
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

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{Analyze, Context, Fragment},
        parser::expression::{ksx::KSX, primitive::Primitive, Expression},
        test::fixture as f,
    };
    use std::collections::HashMap;

    #[test]
    fn text() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::kxc(KSX::Text(String::from("foo")), ()).register(ctx),
            f::kxc(KSX::Text(String::from("foo")), 0)
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![(0, Fragment::KSX(KSX::Text(String::from("foo"))))])
        );
    }

    #[test]
    fn inline() {
        let ctx = &mut Context::new();

        assert_eq!(
            f::kxc(
                KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), ())),
                (),
            )
            .register(ctx),
            f::kxc(
                KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                1,
            )
        );

        assert_eq!(
            ctx.fragments,
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
        let ctx = &mut Context::new();

        assert_eq!(
            f::kxc(
                KSX::Fragment(vec![f::kxc(
                    KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), ())),
                    (),
                )]),
                (),
            )
            .register(ctx),
            f::kxc(
                KSX::Fragment(vec![f::kxc(
                    KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                    1,
                )]),
                2,
            )
        );

        assert_eq!(
            ctx.fragments,
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
        let ctx = &mut Context::new();

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
            .register(ctx),
            f::kxc(
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
                1,
            )
        );

        assert_eq!(
            ctx.fragments,
            HashMap::from_iter(vec![
                (
                    0,
                    Fragment::Expression(Expression::Primitive(Primitive::Nil))
                ),
                (
                    1,
                    Fragment::KSX(KSX::ClosedElement(
                        String::from("Foo"),
                        vec![
                            (String::from("bar"), None),
                            (String::from("fizz"), Some(0),),
                        ],
                    ))
                ),
            ])
        );
    }

    #[test]
    fn open_element() {
        let ctx = &mut Context::new();

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
            .register(ctx),
            f::kxc(
                KSX::OpenElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                        ),
                    ],
                    vec![f::kxc(
                        KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), 1)),
                        2,
                    )],
                    String::from("Foo"),
                ),
                3,
            )
        );

        assert_eq!(
            ctx.fragments,
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
                        vec![
                            (String::from("bar"), None),
                            (String::from("fizz"), Some(0),),
                        ],
                        vec![2],
                        String::from("Foo"),
                    ))
                ),
            ])
        );
    }
}
