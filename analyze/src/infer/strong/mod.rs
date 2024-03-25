mod arithmetic;
mod data;
mod function_result;
mod inherit;
mod module;
mod property;
mod reference;
mod state;

use super::{
    weak::{self, Inference},
    NodeDescriptor,
};
use crate::Context;
use data::{Action, Data};
pub use data::{Output, Result};
use kore::invariant;
use state::State;

pub fn infer_types(ctx: &Context, weak: weak::Result) -> Result {
    let mut state = State::from_weak(weak);

    while !state.is_done() {
        state = partial_infer_types(ctx, state);
    }

    state.into_result()
}

fn partial_infer_types<'a>(ctx: &Context, prev: State<'a>) -> State<'a> {
    let (remaining, mut next) = State::next(prev);
    let remaining_count = remaining.len();

    for node in remaining {
        let action = match &node {
            // capture local types known during this pass
            NodeDescriptor {
                weak: weak::Data::Local(local),
                ..
            } => Action::Infer(Data::Local(local.clone())),

            // capture inherited types
            NodeDescriptor {
                kind,
                weak: weak::Data::Inherit(from_id),
                ..
            } => inherit::inherit(&next, *from_id, kind),

            // capture inherited types of a particular source kind
            // used to infer a value's type from a type expression
            NodeDescriptor {
                weak: weak::Data::InheritKind(from_id, from_kind),
                ..
            } => inherit::inherit(&next, *from_id, from_kind),

            // capture the type referenced by an identifier
            NodeDescriptor {
                weak: weak::Data::Infer(Inference::Reference(name)),
                ..
            } => reference::infer(&next, name, &node),

            // capture the type of dynamic binary operations
            NodeDescriptor {
                weak: weak::Data::Infer(weak::Inference::Arithmetic(lhs, rhs)),
                ..
            } => arithmetic::infer(&next, *lhs, *rhs),

            // capture the type of a property by name
            NodeDescriptor {
                kind,
                weak: weak::Data::Infer(weak::Inference::Property(lhs, property)),
                ..
            } => property::infer(&next, lhs, property, kind),

            // capture the result of calling a function
            NodeDescriptor {
                kind,
                weak: weak::Data::Infer(Inference::FunctionResult(x)),
                ..
            } => function_result::infer(&next, *x, kind),

            // capture the result of a module declaration
            NodeDescriptor {
                weak: weak::Data::Infer(Inference::Module(declarations)),
                ..
            } => module::infer(&next, declarations),

            // capture a type imported from another file
            NodeDescriptor {
                weak: weak::Data::Infer(Inference::Import(import)),
                ..
            } => {
                unimplemented!("import inference not implemented")
                // let current_path = ctx.namespace.to_path("kn");
                // let import_reference = ModuleReference::from_import(current_path, &import);
                // let module = ctx.modules.get(&import_reference);

                // module.map(Action::Infer).unwrap_or_else(|| {
                //     invariant!(
                //         "module could not be found with reference {}",
                //         import_reference.to_path("kn").display()
                //     )
                // })
            }

            NodeDescriptor {
                weak: weak::Data::Infer(Inference::Parameter),
                ..
            } => unimplemented!("parameter inference not implemented"),
        };

        match action {
            Action::Infer(x) => {
                next.types.insert(node.id, (node.kind, Ok(x)));
            }

            Action::Raise(x) => {
                next.types.insert(node.id, (node.kind, Err(x)));
            }

            Action::Inherit(from_id) => next.nodes.push(node.into_inherit_from(from_id)),

            Action::Skip => next.nodes.push(node),
        }
    }

    if next.nodes.len() == remaining_count {
        invariant!(
            "analysis failed to determine all types: {nodes:?}",
            nodes = next.nodes
        );
    }

    next
}
