(define-module (miny)
  #:export (miny)

  #:use-module (guix gexp)
  #:use-module (guix packages)
  #:use-module (guix git-download)
  #:use-module (guix build-system gnu)
  #:use-module ((guix licenses)
                #:prefix license:)
  #:use-module (gnu packages gl))


(define* (git-checkout-archive name git-dir #:optional (commit "HEAD"))
  (computed-file
   (string-append name ".tar.gz")
   (with-imported-modules
	'((guix build utils))
	#~(let ((git #$(file-append (@ (gnu packages version-control) git) "/bin/git")))
		(use-modules (guix build utils))
		(symlink #$(local-file (string-append git-dir "/.git")
							   (string-append name ".git")
							   #:recursive? #t)
				 ".git")
		(invoke git "archive" #$commit "-o" #$output)))))

(define miny
  (package
    (name "miny")
    (version "0.6.0")
    (source (git-checkout-archive name (dirname (current-filename))))
    (build-system gnu-build-system)
    (arguments
     `(#:phases (modify-phases %standard-phases
                  (delete 'configure)
                  (replace 'install
                    (lambda* (#:key outputs #:allow-other-keys)
                      (let* ((out (assoc-ref outputs "out"))
                             (bin (string-append out "/bin")))
                        (install-file "miny" bin)))))
       #:tests? #f))
    (inputs (list freeglut))
    (home-page "https://github.com/spacecamper/miny")
    (synopsis "Minesweeper")
    (description "Minesweeper")
    (license license:expat)))
miny
