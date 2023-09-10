use super::{expression, Context};
use crate::parser::{
    expression::{
        ksx::{KSXNode, KSX},
        ExpressionNode,
    },
    position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

pub fn analyze_ksx<T>(x: KSXNode<T, ()>, ctx: &mut Context) -> KSXNode<T, i32>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let analyze_attributes = |xs: Vec<(String, Option<ExpressionNode<T, ()>>)>,
                              ctx: &mut Context| {
        xs.into_iter()
            .map(|(key, value)| (key, value.map(|x| expression::analyze_expression(x, ctx))))
            .collect::<Vec<_>>()
    };

    let analyze_children = |xs: Vec<KSXNode<T, ()>>, ctx: &mut Context| {
        xs.into_iter()
            .map(|x| analyze_ksx(x, ctx))
            .collect::<Vec<_>>()
    };

    KSXNode(
        x.0.map(|x| match x {
            KSX::Text(x) => KSX::Text(x),

            KSX::Inline(x) => KSX::Inline(expression::analyze_expression(x, ctx)),

            KSX::Fragment(children) => KSX::Fragment(analyze_children(children, ctx)),

            KSX::ClosedElement(tag, attributes) => {
                KSX::ClosedElement(tag, analyze_attributes(attributes, ctx))
            }

            KSX::OpenElement(start_tag, attributes, children, end_tag) => KSX::OpenElement(
                start_tag,
                analyze_attributes(attributes, ctx),
                analyze_children(children, ctx),
                end_tag,
            ),
        })
        .with_context(ctx.generate_id()),
    )
}

#[cfg(test)]
mod tests {
    use super::analyze_ksx;
    use crate::{
        analyzer::Context,
        parser::{
            expression::{
                ksx::{KSXNode, KSX},
                primitive::Primitive,
                Expression, ExpressionNode,
            },
            node::Node,
            range::Range,
            CharStream,
        },
    };

    const RANGE: Range<CharStream> = Range::chars((1, 1), (1, 1));

    #[test]
    fn text() {
        let ctx = &mut Context::new();

        let result = analyze_ksx(
            KSXNode(Node(KSX::Text(String::from("foo")), RANGE, ())),
            ctx,
        );

        assert_eq!(
            result,
            KSXNode(Node(KSX::Text(String::from("foo")), RANGE, 0))
        )
    }

    #[test]
    fn inline() {
        let ctx = &mut Context::new();

        let result = analyze_ksx(
            KSXNode(Node(
                KSX::Inline(ExpressionNode(Node(
                    Expression::Primitive(Primitive::Nil),
                    RANGE,
                    (),
                ))),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            KSXNode(Node(
                KSX::Inline(ExpressionNode(Node(
                    Expression::Primitive(Primitive::Nil),
                    RANGE,
                    0,
                ))),
                RANGE,
                1,
            ))
        )
    }

    #[test]
    fn fragment() {
        let ctx = &mut Context::new();

        let result = analyze_ksx(
            KSXNode(Node(
                KSX::Fragment(vec![KSXNode(Node(
                    KSX::Inline(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        (),
                    ))),
                    RANGE,
                    (),
                ))]),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            KSXNode(Node(
                KSX::Fragment(vec![KSXNode(Node(
                    KSX::Inline(ExpressionNode(Node(
                        Expression::Primitive(Primitive::Nil),
                        RANGE,
                        0,
                    ))),
                    RANGE,
                    1,
                ))]),
                RANGE,
                2,
            ))
        )
    }

    #[test]
    fn closed_element() {
        let ctx = &mut Context::new();

        let result = analyze_ksx(
            KSXNode(Node(
                KSX::ClosedElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(ExpressionNode(Node(
                                Expression::Primitive(Primitive::Nil),
                                RANGE,
                                (),
                            ))),
                        ),
                    ],
                ),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            KSXNode(Node(
                KSX::ClosedElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(ExpressionNode(Node(
                                Expression::Primitive(Primitive::Nil),
                                RANGE,
                                0,
                            ))),
                        ),
                    ],
                ),
                RANGE,
                1,
            ))
        )
    }

    #[test]
    fn open_element() {
        let ctx = &mut Context::new();

        let result = analyze_ksx(
            KSXNode(Node(
                KSX::OpenElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(ExpressionNode(Node(
                                Expression::Primitive(Primitive::Nil),
                                RANGE,
                                (),
                            ))),
                        ),
                    ],
                    vec![KSXNode(Node(
                        KSX::Inline(ExpressionNode(Node(
                            Expression::Primitive(Primitive::Nil),
                            RANGE,
                            (),
                        ))),
                        RANGE,
                        (),
                    ))],
                    String::from("Foo"),
                ),
                RANGE,
                (),
            )),
            ctx,
        );

        assert_eq!(
            result,
            KSXNode(Node(
                KSX::OpenElement(
                    String::from("Foo"),
                    vec![
                        (String::from("bar"), None),
                        (
                            String::from("fizz"),
                            Some(ExpressionNode(Node(
                                Expression::Primitive(Primitive::Nil),
                                RANGE,
                                0,
                            ))),
                        ),
                    ],
                    vec![KSXNode(Node(
                        KSX::Inline(ExpressionNode(Node(
                            Expression::Primitive(Primitive::Nil),
                            RANGE,
                            1,
                        ))),
                        RANGE,
                        2,
                    ))],
                    String::from("Foo"),
                ),
                RANGE,
                3,
            ))
        )
    }
}
