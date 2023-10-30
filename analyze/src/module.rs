use super::{
    context::{NodeContext, StrongContext},
    fragment::Fragment,
    infer::{
        strong::{Strong, ToStrong},
        weak::{ToWeak, Weak, WeakRef},
    },
    register::{Identify, Register, ToFragment},
    RefKind, ScopeContext,
};
use lang::ast::{Module, ModuleNode, ModuleNodeValue};

impl<R> Identify<ModuleNodeValue<R, NodeContext>> for ModuleNodeValue<R, ()>
where
    R: Copy,
{
    fn identify(&self, ctx: &ScopeContext) -> ModuleNodeValue<R, NodeContext> {
        self.map(&|x| x.register(ctx), &|x| x.register(ctx))
    }
}

impl<R> ToFragment for ModuleNodeValue<R, NodeContext>
where
    R: Copy,
{
    fn to_fragment(&self) -> Fragment {
        Fragment::Module(self.map(&|x| *x.node().id(), &|x| *x.node().id()))
    }
}

impl<R> Register for ModuleNode<R, ()>
where
    R: Copy,
{
    type Node = ModuleNode<R, NodeContext>;
    type Value<C> = ModuleNodeValue<R, C>;

    fn register(&self, ctx: &ScopeContext) -> ModuleNode<R, NodeContext> {
        let value = self.0.identify(ctx);
        let id = ctx.add_fragment(&value);

        ModuleNode(value, id)
    }
}

impl ToWeak for Module<usize, usize> {
    fn to_weak(&self) -> WeakRef {
        (RefKind::Mixed, Weak::Infer)
    }
}

impl<R> ToStrong<ModuleNode<R, Strong>> for ModuleNode<R, NodeContext>
where
    R: Copy,
{
    fn to_strong(&self, ctx: &StrongContext) -> ModuleNode<R, Strong> {
        ModuleNode(
            self.0.map(&|x| x.to_strong(ctx), &|x| x.to_strong(ctx)),
            ctx.resolve(self.id()).clone(),
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        context::{FragmentMap, NodeContext},
        fragment::Fragment,
        register::Register,
        test::fixture as f,
        test::fixture::{file_ctx, scope_ctx},
    };
    use kore::assert_eq;
    use lang::ast::{
        Expression, Import, ImportSource, ImportTarget, Module, ModuleNode, Primitive,
        TypeExpression,
    };

    #[test]
    fn module() {
        let file = &file_ctx();
        let scope = &mut scope_ctx(file);

        assert_eq!(
            f::n::mr(Module::new(
                vec![f::n::i(Import {
                    source: ImportSource::Root,
                    path: vec![String::from("bar"), String::from("fizz")],
                    aliases: Some(vec![(ImportTarget::Module, Some(String::from("Fizz")))]),
                })],
                vec![f::n::d(f::a::const_(
                    "BUZZ",
                    Some(f::n::tx(TypeExpression::Nil)),
                    f::n::x(Expression::Primitive(Primitive::Nil))
                ))],
            ))
            .register(scope),
            ModuleNode(
                Module::new(
                    vec![f::n::ic(
                        Import {
                            source: ImportSource::Root,
                            path: vec![String::from("bar"), String::from("fizz")],
                            aliases: Some(vec![(ImportTarget::Module, Some(String::from("Fizz")))]),
                        },
                        NodeContext::new(0, vec![0])
                    )],
                    vec![f::n::dc(
                        f::a::const_(
                            "BUZZ",
                            Some(f::n::txc(
                                TypeExpression::Nil,
                                NodeContext::new(1, vec![0, 2])
                            )),
                            f::n::xc(
                                Expression::Primitive(Primitive::Nil),
                                NodeContext::new(2, vec![0, 2])
                            )
                        ),
                        NodeContext::new(3, vec![0]),
                    )],
                ),
                NodeContext::new(4, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            FragmentMap::from_iter(vec![
                (
                    0,
                    (
                        vec![0],
                        Fragment::Import(Import {
                            source: ImportSource::Root,
                            path: vec![String::from("bar"), String::from("fizz")],
                            aliases: Some(vec![(ImportTarget::Module, Some(String::from("Fizz")))]),
                        })
                    )
                ),
                (
                    1,
                    (vec![0, 2], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    2,
                    (
                        vec![0, 2],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    3,
                    (
                        vec![0],
                        Fragment::Declaration(f::a::const_("BUZZ", Some(1), 2))
                    )
                ),
                (
                    4,
                    (vec![0], Fragment::Module(Module::new(vec![0], vec![3])))
                ),
            ])
        );
    }
}
