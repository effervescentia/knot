use super::ScopeContext;
use crate::{
    analyzer::{
        context::NodeContext,
        register::{Identify, Register},
    },
    ast::KSXNodeValue,
};

impl Identify<KSXNodeValue<NodeContext>> for KSXNodeValue<()> {
    fn identify(&self, ctx: &ScopeContext) -> KSXNodeValue<NodeContext> {
        self.map(&mut |x| x.register(ctx), &mut |x| x.register(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, register::Identify},
        ast::{Expression, ExpressionNode, KSXNode, Primitive, KSX},
        test::fixture as f,
    };

    #[test]
    fn text() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            KSX::<ExpressionNode<()>, KSXNode<()>>::Text(String::from("foo")).identify(scope),
            KSX::Text(String::from("foo"))
        );
    }

    #[test]
    fn inline() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

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
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

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
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

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
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

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
