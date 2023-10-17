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
use crate::{
    ast::{Module, ModuleNode, ModuleNodeValue},
    common::position::Decrement,
};
use combine::Stream;
use std::fmt::Debug;

impl<T> ModuleNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    pub fn id(&self) -> &usize {
        self.1.id()
    }
}

impl<T> Identify<ModuleNodeValue<T, NodeContext>> for ModuleNodeValue<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn identify(&self, ctx: &ScopeContext) -> ModuleNodeValue<T, NodeContext> {
        self.map(&|x| x.register(ctx))
    }
}

impl<T> ToFragment for ModuleNodeValue<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_fragment<'a>(&'a self) -> Fragment {
        Fragment::Module(self.map(&|x| *x.node().id()))
    }
}

impl<T> Register for ModuleNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    type Node = ModuleNode<T, NodeContext>;
    type Value<C> = ModuleNodeValue<T, C>;

    fn register(&self, ctx: &ScopeContext) -> ModuleNode<T, NodeContext> {
        let value = self.0.identify(ctx);
        let id = ctx.add_fragment(&value);

        ModuleNode(value, id)
    }
}

impl ToWeak for Module<usize> {
    fn to_weak(&self) -> WeakRef {
        (RefKind::Mixed, Weak::Infer)
    }
}

impl<T> ToStrong<ModuleNode<T, Strong>> for ModuleNode<T, NodeContext>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn to_strong(&self, ctx: &StrongContext) -> ModuleNode<T, Strong> {
        ModuleNode(
            self.0.map(&|x| x.to_strong(ctx)),
            ctx.resolve(self.id()).clone(),
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        analyzer::{
            context::{FragmentMap, NodeContext},
            fragment::Fragment,
            register::Register,
        },
        ast::{
            Expression, Import, ImportSource, ImportTarget, Module, ModuleNode, Primitive,
            TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn module() {
        let file = &f::file_ctx();
        let scope = &mut f::scope_ctx(file);

        assert_eq!(
            f::n::mr(Module::new(
                vec![Import {
                    source: ImportSource::Root,
                    path: vec![String::from("bar"), String::from("fizz")],
                    aliases: Some(vec![(ImportTarget::Module, Some(String::from("Fizz")))]),
                }],
                vec![f::n::d(f::a::const_(
                    "BUZZ",
                    Some(f::n::tx(TypeExpression::Nil)),
                    f::n::x(Expression::Primitive(Primitive::Nil))
                ))],
            ))
            .register(scope),
            ModuleNode(
                Module::new(
                    vec![Import {
                        source: ImportSource::Root,
                        path: vec![String::from("bar"), String::from("fizz")],
                        aliases: Some(vec![(ImportTarget::Module, Some(String::from("Fizz")))]),
                    }],
                    vec![f::n::dc(
                        f::a::const_(
                            "BUZZ",
                            Some(f::n::txc(
                                TypeExpression::Nil,
                                NodeContext::new(0, vec![0, 1])
                            )),
                            f::n::xc(
                                Expression::Primitive(Primitive::Nil),
                                NodeContext::new(1, vec![0, 1])
                            )
                        ),
                        NodeContext::new(2, vec![0]),
                    )],
                ),
                NodeContext::new(3, vec![0]),
            )
        );

        assert_eq!(
            scope.file.borrow().fragments,
            FragmentMap::from_iter(vec![
                (
                    0,
                    (vec![0, 1], Fragment::TypeExpression(TypeExpression::Nil))
                ),
                (
                    1,
                    (
                        vec![0, 1],
                        Fragment::Expression(Expression::Primitive(Primitive::Nil))
                    )
                ),
                (
                    2,
                    (
                        vec![0],
                        Fragment::Declaration(f::a::const_("BUZZ", Some(0), 1))
                    )
                ),
                (
                    3,
                    (
                        vec![0],
                        Fragment::Module(Module::new(
                            vec![Import {
                                source: ImportSource::Root,
                                path: vec![String::from("bar"), String::from("fizz")],
                                aliases: Some(vec![(
                                    ImportTarget::Module,
                                    Some(String::from("Fizz"))
                                )]),
                            }],
                            vec![2],
                        ))
                    )
                ),
            ])
        );
    }
}
