window.AgeRunWalkthroughDeck = {
    init({ repositoryBaseUrl, repositoryPathPattern, slides }) {
        let currentSlideIndex = 0;
        let isSidebarOpen = false;

        const slideElement = document.getElementById("slide");
        const counterElement = document.getElementById("slide-counter");
        const prevButton = document.getElementById("prev-button");
        const nextButton = document.getElementById("next-button");
        const menuButton = document.getElementById("menu-button");
        const sidebarCloseButton = document.getElementById("sidebar-close");
        const sidebarBackdrop = document.getElementById("sidebar-backdrop");
        const sidebarListElement = document.getElementById("slide-nav-list");

        function createRepositoryLink(path) {
            const link = document.createElement("a");
            link.className = "source-link";
            link.href = `${repositoryBaseUrl}${path}`;
            link.target = "_blank";
            link.rel = "noopener noreferrer";
            link.textContent = path;
            return link;
        }

        function linkifyRepositoryPaths(root) {
            const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT);
            const textNodes = [];

            while (walker.nextNode()) {
                const textNode = walker.currentNode;
                if (textNode.parentElement?.closest("a")) {
                    continue;
                }
                textNodes.push(textNode);
            }

            textNodes.forEach((textNode) => {
                const pattern = new RegExp(repositoryPathPattern.source, "g");
                const text = textNode.textContent;
                let lastIndex = 0;
                let match;
                let changed = false;
                const fragment = document.createDocumentFragment();

                while ((match = pattern.exec(text)) !== null) {
                    changed = true;
                    fragment.append(text.slice(lastIndex, match.index));
                    fragment.append(createRepositoryLink(match[0]));
                    lastIndex = match.index + match[0].length;
                }

                if (!changed) {
                    return;
                }

                fragment.append(text.slice(lastIndex));
                textNode.parentNode.replaceChild(fragment, textNode);
            });
        }

        function setSidebarOpen(nextState) {
            isSidebarOpen = nextState;
            document.body.classList.toggle("sidebar-open", isSidebarOpen);
            menuButton.setAttribute("aria-expanded", String(isSidebarOpen));
            sidebarBackdrop.hidden = !isSidebarOpen;
        }

        function renderSidebar() {
            sidebarListElement.innerHTML = "";

            slides.forEach((slide, index) => {
                const button = document.createElement("button");
                button.type = "button";
                button.className = "nav-link";
                if (index === currentSlideIndex) {
                    button.classList.add("active");
                    button.setAttribute("aria-current", "page");
                }

                button.innerHTML = `
                    <span class="nav-link-title">${index + 1}. ${slide.title}</span>
                    <span class="nav-link-subtitle">${slide.subtitle}</span>
                `;
                button.addEventListener("click", () => goToSlide(index));
                sidebarListElement.append(button);
            });
        }

        function renderSlide() {
            const slide = slides[currentSlideIndex];

            slideElement.innerHTML = `
                <h2 class="slide-title">${slide.title}</h2>
                <p class="slide-subtitle">${slide.subtitle}</p>
                ${slide.body}
            `;
            linkifyRepositoryPaths(slideElement);
            renderSidebar();

            counterElement.textContent = `${currentSlideIndex + 1} / ${slides.length}`;
            prevButton.disabled = currentSlideIndex === 0;
            nextButton.disabled = currentSlideIndex === slides.length - 1;
        }

        function goToSlide(index) {
            if (index < 0 || index >= slides.length) {
                return;
            }

            currentSlideIndex = index;
            renderSlide();
            setSidebarOpen(false);
        }

        function goToPreviousSlide() {
            goToSlide(currentSlideIndex - 1);
        }

        function goToNextSlide() {
            goToSlide(currentSlideIndex + 1);
        }

        prevButton.addEventListener("click", goToPreviousSlide);
        nextButton.addEventListener("click", goToNextSlide);
        menuButton.addEventListener("click", () => setSidebarOpen(!isSidebarOpen));
        sidebarCloseButton.addEventListener("click", () => setSidebarOpen(false));
        sidebarBackdrop.addEventListener("click", () => setSidebarOpen(false));

        window.addEventListener("resize", () => {
            if (window.innerWidth > 900 && isSidebarOpen) {
                setSidebarOpen(false);
            }
        });

        document.addEventListener("keydown", (event) => {
            if (["ArrowLeft", "k", "K"].includes(event.key)) {
                goToPreviousSlide();
            }

            if (["ArrowRight", "j", "J"].includes(event.key)) {
                goToNextSlide();
            }

            if (event.key === "Escape" && isSidebarOpen) {
                setSidebarOpen(false);
            }
        });

        renderSlide();
        setSidebarOpen(false);
    }
};
