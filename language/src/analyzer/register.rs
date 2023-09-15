use super::context::{NodeContext, ScopeContext};

pub trait Identify<R> {
    fn identify(self, ctx: &mut ScopeContext) -> R;
}

pub trait ToRef<R> {
    fn to_ref<'a>(&'a self) -> R;
}

pub trait Register: Sized {
    type Ref;
    type Node;
    type Value<C>: Identify<Self::Value<NodeContext>> + ToRef<Self::Ref>;

    fn register(self, ctx: &mut ScopeContext) -> Self::Node;

    // fn identify(value: Self::Value<()>, ctx: &mut ScopeContext) -> Self::Value<NodeContext>;

    // fn to_ref<'a>(value: &'a Self::Value<NodeContext>) -> Self::Ref;
}
