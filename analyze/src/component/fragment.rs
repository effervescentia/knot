use crate::{context::NodeContext, fragment::Fragment, register::ToFragment};
use lang::ast::{AstNode, KSXNodeValue};

impl<R> ToFragment for KSXNodeValue<R, NodeContext>
where
    R: Copy,
{
    fn to_fragment(&self) -> Fragment {
        Fragment::KSX(self.map(&mut |x| *x.node().id(), &mut |x| *x.node().id()))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::NodeContext, fragment::Fragment, register::ToFragment, test::fixture as f,
    };
    use kore::str;
    use lang::ast::{Expression, ExpressionNode, KSXNode, Primitive, KSX};
    use parse::Range;

    #[test]
    fn text() {
        assert_eq!(
            KSX::<ExpressionNode<Range, NodeContext>, KSXNode<Range, NodeContext>>::Text(str!(
                "foo"
            ))
            .to_fragment(),
            Fragment::KSX(KSX::Text(str!("foo")))
        );
    }

    #[test]
    fn inline() {
        assert_eq!(
            KSX::Inline(f::n::xc(
                Expression::Primitive(Primitive::Nil),
                NodeContext::new(0, vec![0])
            ))
            .to_fragment(),
            Fragment::KSX(KSX::Inline(0))
        );
    }

    #[test]
    fn fragment() {
        assert_eq!(
            KSX::Fragment(vec![f::n::kxc(
                KSX::Inline(f::n::xc(
                    Expression::Primitive(Primitive::Nil),
                    NodeContext::new(0, vec![0])
                )),
                NodeContext::new(1, vec![0]),
            )])
            .to_fragment(),
            Fragment::KSX(KSX::Fragment(vec![1]))
        );
    }

    #[test]
    fn closed_element() {
        assert_eq!(
            KSX::ClosedElement(
                str!("Foo"),
                vec![
                    (str!("bar"), None),
                    (
                        str!("fizz"),
                        Some(f::n::xc(
                            Expression::Primitive(Primitive::Nil),
                            NodeContext::new(0, vec![0])
                        )),
                    ),
                ],
            )
            .to_fragment(),
            Fragment::KSX(KSX::ClosedElement(
                str!("Foo"),
                vec![(str!("bar"), None), (str!("fizz"), Some(0)),],
            ))
        );
    }

    #[test]
    fn open_element() {
        assert_eq!(
            KSX::OpenElement(
                str!("Foo"),
                vec![
                    (str!("bar"), None),
                    (
                        str!("fizz"),
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
                str!("Foo"),
            )
            .to_fragment(),
            Fragment::KSX(KSX::OpenElement(
                str!("Foo"),
                vec![(str!("bar"), None), (str!("fizz"), Some(0))],
                vec![2],
                str!("Foo"),
            ))
        );
    }
}
