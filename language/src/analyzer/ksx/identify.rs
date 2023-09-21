use super::ScopeContext;
use crate::{
    analyzer::{
        context::NodeContext,
        register::{Identify, Register},
    },
    ast::{
        expression::ExpressionNode,
        ksx::{self, KSXNode, KSX},
    },
    common::position::Decrement,
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

impl<T> Identify<ksx::NodeValue<T, NodeContext>> for ksx::NodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(self, ctx: &mut ScopeContext) -> ksx::NodeValue<T, NodeContext> {
        match self {
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
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, register::Identify},
        ast::{
            expression::{Expression, ExpressionNode, Primitive},
            ksx::{KSXNode, KSX},
        },
        parser::CharStream,
        test::fixture as f,
    };

    #[test]
    fn text() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            KSX::<ExpressionNode<CharStream<'static>, ()>, KSXNode<CharStream<'static>, ()>>::Text(
                String::from("foo")
            )
            .identify(scope),
            KSX::Text(String::from("foo"))
        );
    }

    #[test]
    fn inline() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            KSX::Inline(f::n::x(Expression::Primitive(Primitive::Nil))).identify(scope),
            KSX::Inline(f::n::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            ))
        );
    }

    #[test]
    fn fragment() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            KSX::Fragment(vec![f::n::kx(KSX::Inline(f::n::x(Expression::Primitive(
                Primitive::Nil
            ))))])
            .identify(scope),
            KSX::Fragment(vec![f::n::kxc(
                KSX::Inline(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
                NodeContext::new(1, vec![0]),
            )])
        );
    }

    #[test]
    fn closed_element() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            KSX::ClosedElement(
                String::from("Foo"),
                vec![
                    (String::from("bar"), None),
                    (
                        String::from("fizz"),
                        Some(f::n::x(Expression::Primitive(Primitive::Nil))),
                    ),
                ],
            )
            .identify(scope),
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
            )
        );
    }

    #[test]
    fn open_element() {
        let file = &f::f_ctx();
        let scope = &mut f::s_ctx(file);

        assert_eq!(
            KSX::OpenElement(
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
            )
            .identify(scope),
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
            )
        );
    }
}
