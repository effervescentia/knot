mod data;
mod to_weak;

pub use data::{Data, Inference, Result, Weak};
use lang::FragmentMap;

pub trait ToWeak {
    fn to_weak(&self) -> Weak;
}

pub fn infer_types(fragments: &FragmentMap) -> Result {
    let mut result = Result::new(fragments);

    for (id, (scope, fragment)) in fragments {
        result.types.insert(*id, fragment.to_weak());

        if let Some(name) = fragment.to_binding() {
            result
                .bindings
                .0
                .entry((scope.clone(), name))
                .or_default()
                .insert(*id);
        }
    }

    result
}
