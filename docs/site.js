(function () {
    function updateScrollCue(ref_element) {
        const hasOverflow = ref_element.scrollHeight > ref_element.clientHeight + 2;
        const canScrollUp = ref_element.scrollTop > 2;
        const canScrollDown = ref_element.scrollTop + ref_element.clientHeight < ref_element.scrollHeight - 2;

        ref_element.classList.toggle("is-scrollable", hasOverflow);
        ref_element.classList.toggle("can-scroll-up", hasOverflow && canScrollUp);
        ref_element.classList.toggle("can-scroll-down", hasOverflow && canScrollDown);
    }

    function attachScrollCue(ref_element) {
        let animationFrame = 0;

        function scheduleUpdate() {
            if (animationFrame) {
                return;
            }

            animationFrame = window.requestAnimationFrame(function () {
                animationFrame = 0;
                updateScrollCue(ref_element);
            });
        }

        ref_element.addEventListener("scroll", scheduleUpdate, { passive: true });
        window.addEventListener("resize", scheduleUpdate);

        const resizeObserver = new ResizeObserver(scheduleUpdate);
        resizeObserver.observe(ref_element);

        const mutationObserver = new MutationObserver(scheduleUpdate);
        mutationObserver.observe(ref_element, {
            childList: true,
            subtree: true,
            characterData: true,
            attributes: true,
        });

        scheduleUpdate();
    }

    function initializeScrollCues() {
        const paneElements = document.querySelectorAll(".viewport-sidebar, .viewport-content");
        paneElements.forEach(attachScrollCue);
    }

    if (document.readyState === "loading") {
        document.addEventListener("DOMContentLoaded", initializeScrollCues);
    } else {
        initializeScrollCues();
    }
})();
