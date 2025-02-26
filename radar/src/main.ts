import { mount } from "svelte";
import App from "./Radar.svelte";

const app = mount(App, {
    target: document.getElementById("app")!,
});

export default app;
