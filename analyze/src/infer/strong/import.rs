use super::data::{Action, Type};
use crate::Context;
use kore::invariant;
use lang::{ast, Namespace};

pub fn infer(ctx: &Context, source: &ast::ImportSource, path: &[String]) -> Action {
    let current_path = ctx.namespace.to_path("kn");
    let namespace = Namespace::from_path(current_path, source, path);
    let module = ctx.modules.resolve_module(&namespace);

    module
        .map(|x| Action::Infer(Type::Inherit(x.0)))
        .unwrap_or_else(|| {
            invariant!(
                "module could not be found with path {}",
                namespace.to_path("kn").display()
            )
        })
}
