use super::data::{Action, Type};
use crate::Context;
use kore::invariant;
use lang::{ast, Namespace};
use std::rc::Rc;

pub fn infer(ctx: &Context, source: ast::ImportSource, path: Vec<String>) -> Action {
    let current_path = ctx.namespace.to_path("kn");
    let namespace = Namespace::from_path(current_path, &source, &path);
    let module = ctx.modules.get(&namespace);

    module
        .map(|x| Action::Infer(Type::Remote(Rc::clone(x))))
        .unwrap_or_else(|| {
            invariant!(
                "module could not be found with path {}",
                namespace.to_path("kn").display()
            )
        })
}
