use crate::{
    analyzer::{
        context::{NodeContext, ScopeContext},
        register::{Identify, Register},
    },
    ast::TypeExpressionNodeValue,
};

impl Identify<TypeExpressionNodeValue<NodeContext>> for TypeExpressionNodeValue<()> {
    fn identify(&self, ctx: &ScopeContext) -> TypeExpressionNodeValue<NodeContext> {
        self.map(&|x| x.register(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{context::NodeContext, register::Identify},
        ast::{TypeExpression, TypeExpressionNode},
        test::fixture as f,
    };

    #[test]
    fn primitive() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            TypeExpression::<TypeExpressionNode<()>>::Nil.identify(scope),
            TypeExpression::Nil
        );
    }

    #[test]
    fn identifier() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            TypeExpression::<TypeExpressionNode<()>>::Identifier(String::from("foo"))
                .identify(scope),
            TypeExpression::Identifier(String::from("foo"))
        );
    }

    #[test]
    fn group() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            TypeExpression::Group(Box::new(f::n::tx(TypeExpression::Nil))).identify(scope),
            TypeExpression::Group(Box::new(f::n::txc(
                TypeExpression::Nil,
                NodeContext::new(0, vec![0])
            )))
        );
    }

    #[test]
    fn dot_access() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            TypeExpression::DotAccess(
                Box::new(f::n::tx(TypeExpression::Nil)),
                String::from("foo"),
            )
            .identify(scope),
            TypeExpression::DotAccess(
                Box::new(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                String::from("foo"),
            )
        );
    }

    #[test]
    fn function() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            TypeExpression::Function(
                vec![f::n::tx(TypeExpression::Nil), f::n::tx(TypeExpression::Nil),],
                Box::new(f::n::tx(TypeExpression::Nil)),
            )
            .identify(scope),
            TypeExpression::Function(
                vec![
                    f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0])),
                    f::n::txc(TypeExpression::Nil, NodeContext::new(1, vec![0])),
                ],
                Box::new(f::n::txc(TypeExpression::Nil, NodeContext::new(2, vec![0]))),
            )
        );
    }
}
