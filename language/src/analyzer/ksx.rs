use super::{reference::ToRef, Context, Register};
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

fn identify_ksx<T>(
    x: KSXNode<T, ()>,
    ctx: &mut Context,
) -> Node<KSX<ExpressionNode<T, usize>, KSXNode<T, usize>>, T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    x.0.map(|x| match x {
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
    })
}

impl<T> Register<KSXNode<T, usize>> for KSXNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn register(self, ctx: &mut Context) -> KSXNode<T, usize> {
        let node = identify_ksx(self, ctx);
        let fragment = Fragment::KSX(node.value().to_ref());
        let id = ctx.register(fragment);

        KSXNode(node.with_context(id))
    }
}

#[cfg(test)]
mod tests {
    // use super::analyze_ksx;
    use crate::{
        analyzer::{Context, Register},
        parser::expression::{ksx::KSX, primitive::Primitive, Expression},
        test::fixture as f,
    };

    #[test]
    fn text() {
        let ctx = &mut Context::new();

        let result = f::kxc(KSX::Text(String::from("foo")), ()).register(ctx);

        assert_eq!(result, f::kxc(KSX::Text(String::from("foo")), 0))
    }

    #[test]
    fn inline() {
        let ctx = &mut Context::new();

        let result = f::kxc(
            KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), ())),
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::kxc(
                KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), 0)),
                1,
            )
        )
    }

    #[test]
    fn fragment() {
        let ctx = &mut Context::new();

        let result = f::kxc(
            KSX::Fragment(vec![f::kxc(
                KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), ())),
                (),
            )]),
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            f::kxc(
                KSX::Fragment(vec![f::kxc(
                    KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), 0,)),
                    1,
                )]),
                2,
            )
        )
    }

    #[test]
    fn closed_element() {
        let ctx = &mut Context::new();

        let result = f::kxc(
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
        .register(ctx);

        assert_eq!(
            result,
            f::kxc(
                KSX::ClosedElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(f::xc(Expression::Primitive(Primitive::Nil), 0,)),
                        ),
                    ],
                ),
                1,
            )
        )
    }

    #[test]
    fn open_element() {
        let ctx = &mut Context::new();

        let result = f::kxc(
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
        .register(ctx);

        assert_eq!(
            result,
            f::kxc(
                KSX::OpenElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(f::xc(Expression::Primitive(Primitive::Nil), 0,)),
                        ),
                    ],
                    vec![f::kxc(
                        KSX::Inline(f::xc(Expression::Primitive(Primitive::Nil), 1,)),
                        2,
                    )],
                    String::from("Foo"),
                ),
                3,
            )
        )
    }
}
