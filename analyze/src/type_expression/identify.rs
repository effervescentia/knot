use crate::{
    context::{NodeContext, ScopeContext},
    register::{Identify, Register},
};
use lang::ast::TypeExpressionNodeValue;

impl<R> Identify<TypeExpressionNodeValue<R, NodeContext>> for TypeExpressionNodeValue<R, ()>
where
    R: Copy,
{
    fn identify(&self, ctx: &ScopeContext) -> TypeExpressionNodeValue<R, NodeContext> {
        self.map(&|x| x.register(ctx))
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::NodeContext,
        register::Identify,
        test::fixture as f,
        test::fixture::{file_ctx, scope_ctx},
    };
    use kore::str;
    use lang::ast::{TypeExpression, TypeExpressionNode};
    use parse::Range;

    #[test]
    fn primitive() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            TypeExpression::<TypeExpressionNode<Range, ()>>::Nil.identify(scope),
            TypeExpression::Nil
        );
    }

    #[test]
    fn identifier() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            TypeExpression::<TypeExpressionNode<Range, ()>>::Identifier(str!("foo"))
                .identify(scope),
            TypeExpression::Identifier(str!("foo"))
        );
    }

    #[test]
    fn group() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

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
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            TypeExpression::DotAccess(Box::new(f::n::tx(TypeExpression::Nil)), str!("foo"),)
                .identify(scope),
            TypeExpression::DotAccess(
                Box::new(f::n::txc(TypeExpression::Nil, NodeContext::new(0, vec![0]))),
                str!("foo"),
            )
        );
    }

    #[test]
    fn function() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

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
